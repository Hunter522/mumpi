#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <portaudio.h>
#include <mumlib/Transport.hpp>
#include "MumpiCallback.hpp"
#include "RingBuffer.hpp"

#define SAMPLE_RATE (48000)
#define NUM_CHANNELS (1)
#define FRAMES_PER_BUFFER (512)

static log4cpp::Appender *appender = new log4cpp::OstreamAppender("console", &std::cout);
static log4cpp::Category& logger = log4cpp::Category::getRoot();
static volatile sig_atomic_t sig_caught = 0;
static bool mumble_thread_run_flag = true;
static bool input_consumer_thread_run_flag = true;

/**
 * Signal interrupt handler
 *
 * @param signal the signal
 */
static void sigHandler(int signal) {
	logger.info("caught signal %d", signal);
	sig_caught = signal;
}

/**
 * Simple data structure for storing audio sample data
 */
struct PaData {
	std::shared_ptr<RingBuffer<int16_t>> rec_buf;	// recording ring buffer
	std::shared_ptr<RingBuffer<int16_t>> out_buf;	// output ring buffer
};

/**
 * Record callback for PortAudio engine. This gets called when audio input is
 * available. This will simply take the data from the input buffer and store
 * it in a ring buffer that we allocated. That data will then be consumed by
 * another thread.
 *
 * @param  inputBuffer     [description]
 * @param  outputBuffer    [description]
 * @param  framesPerBuffer [description]
 * @param  timeInfo        [description]
 * @param  statusFlags     [description]
 * @param  userData        [description]
 * @return                 [description]
 */
static int paRecordCallback(const void *inputBuffer,
                            void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData ) {
	int result = paContinue;
	// cast the pointers to the appropriate types
	const PaData *pa_data = (const PaData*) userData;
	int16_t *input_buffer = (int16_t*) inputBuffer;
	(void) outputBuffer;
	(void) timeInfo;
	(void) statusFlags;

	if(inputBuffer != NULL) {
		// fill ring buffer with samples
		pa_data->rec_buf->push(input_buffer, 0, framesPerBuffer * NUM_CHANNELS);
	} else {
		// fill ring buffer with silence
		for(int i = 0; i < framesPerBuffer * NUM_CHANNELS; i += NUM_CHANNELS) {
			for(int j = 0; j < NUM_CHANNELS; j++) {
				pa_data->rec_buf->push(0);
			}
		}
	}

	return result;
}

/**
 * Output callback for PortAudio engine. This gets called when audio output is
 * ready to be sent. This will simply consume data from a ring buffer that we
 * allocated which is being filled by another thread.
 *
 * @param  inputBuffer     [description]
 * @param  outputBuffer    [description]
 * @param  framesPerBuffer [description]
 * @param  timeInfo        [description]
 * @param  statusFlags     [description]
 * @param  userData        [description]
 * @return                 [description]
 */
static int paOutputCallback(const void *inputBuffer,
                            void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData ) {
	int result = paContinue;
	// cast the pointers to the appropriate types
	const PaData *pa_data = (const PaData*) userData;
	int16_t *output_buffer = (int16_t*) outputBuffer;
	(void) inputBuffer;
	(void) timeInfo;
	(void) framesPerBuffer;
	(void) statusFlags;

	// output pcm data to PortAudio's output_buffer by reading from our ring buffer
	// if we dont have enough samples in our ring buffer, we have to still supply 0s to the output_buffer
	const size_t requested_samples = (framesPerBuffer * NUM_CHANNELS);
	const size_t available_samples = pa_data->out_buf->getRemaining();
	if(requested_samples > available_samples) {
		pa_data->out_buf->top(output_buffer, 0, available_samples);
		for(size_t i = available_samples; i < requested_samples - available_samples; i++) {
			output_buffer[i] = 0;
		}
	} else {
		pa_data->out_buf->top(output_buffer, 0, requested_samples);
	}

	return result;
}

static unsigned nextPowerOf2(unsigned val) {
    val--;
    val = (val >> 1) | val;
    val = (val >> 2) | val;
    val = (val >> 4) | val;
    val = (val >> 8) | val;
    val = (val >> 16) | val;
    return ++val;
}

/**
 * Displays usage
 */
void help() {
	printf("mumpi - Simple mumble client daemon for the RaspberryPi\n\n");
	printf("Usage:\n");
	printf("mumpi (-s|--server) string (-u|--username) string (-c|--cert) "
	       "string [(-h|--help)] [(-v|--verbose)]\n\n");
	printf("Options:\n");
	printf("-h, --help                Displays this information.\n");
	printf("-v, --verbose             Verbose mode on.\n");
	printf("-s, --server <string>     mumble server IP:PORT. Required.\n");
	printf("-u, --username <username> username. Required.\n");
	exit(1);
}

/**
 * main function
 *
 * Program flow:
 * 1. Parse command line args
 * 2. Init PortAudio engine and open default input and output audio devices
 * 3. Init mumlib client
 * 4. Busy loop until CTRL+C
 * 5. Clean up mumlib client
 * 6. Clean up PortAudio engine
 */
int main(int argc, char *argv[]) {
	bool verbose = false;
	std::string server;
	std::string username;
	int next_option;
	const char* const short_options = "hvs:u:";
	const struct option long_options[] =
	{
		{ "help", 0, NULL, 'h' },
		{ "verbose", 0, NULL, 'v' },
		{ "server", 1, NULL, 's' },
		{ "username", 1, NULL, 'u' },
		{ NULL, 0, NULL, 0 }
	};

	// init logger
	appender->setLayout(new log4cpp::BasicLayout());
	logger.setPriority(log4cpp::Priority::WARN);
	logger.addAppender(appender);

	// parse command line args using getopt
	while(1) {
		// obtain a option
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		if(next_option == -1)
			break;  // no more options

		switch(next_option) {

		case 'h':      // -h or --help
			help();
			break;

		case 'v':      // -v or --verbose
			verbose = true;
			break;

		case 's':      // -s or --server
			server = std::string(optarg);
			break;

		case 'u':      // -u or --username
			username = std::string(optarg);
			break;

		case '?':      // Invalid option
			help();

		case -1:      // No more options
			break;

		default:      // shouldn't happen :-)
			return(1);
		}
	}

	if(verbose)
		logger.setPriority(log4cpp::Priority::INFO);

	// check for mandatory arguments
	if(server.empty() || username.empty()) {
		logger.error("Mandatory arguments not specified");
		help();
	}

	logger.info("Server:        %s", server.c_str());
	logger.info("Username:      %s", username.c_str());

	///////////////////////
	// init audio library
	///////////////////////
	PaError err;
	err = Pa_Initialize();
	if(err != paNoError) {
		logger.error("PortAudio error: %s", Pa_GetErrorText(err));
		exit(-1);
	}

	logger.info(Pa_GetVersionText());

	// init audio I/O streams
	PaStream *input_stream;
	PaStream *output_stream;
	PaData data;
	PaStreamParameters inputParameters;
	PaStreamParameters output_parameters;

	// set ring buffer size to about 500ms
	const size_t MAX_SAMPLES = nextPowerOf2(0.5 * SAMPLE_RATE * NUM_CHANNELS);
	data.rec_buf = std::make_shared<RingBuffer<int16_t>>(MAX_SAMPLES);
	data.out_buf = std::make_shared<RingBuffer<int16_t>>(MAX_SAMPLES);

	inputParameters.device = Pa_GetDefaultInputDevice();
	if (inputParameters.device == paNoDevice) {
		logger.error("No default input device.");
		exit(-1);
	}
	inputParameters.channelCount = NUM_CHANNELS;
	inputParameters.sampleFormat = paInt16;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(&input_stream,         // the input stream
						&inputParameters,      // input params
						NULL,                  // output params
						SAMPLE_RATE,           // sample rate
						FRAMES_PER_BUFFER,     // frames per buffer
						paClipOff,             // we won't output out of range samples so don't bother clipping them
						paRecordCallback,      // PortAudio callback function
						&data);                // data pointer

	if(err != paNoError) {
	    logger.error("Failed to open input stream: %s", Pa_GetErrorText(err));
		exit(-1);
	}

	err = Pa_StartStream(input_stream);
	if(err != paNoError) {
		logger.error("Failed to start input stream: %s", Pa_GetErrorText(err));
		exit(-1);
	}

	output_parameters.device = Pa_GetDefaultOutputDevice();
	if(output_parameters.device == paNoDevice) {
		logger.error("No default output device.");
		exit(-1);
	}
	output_parameters.channelCount = NUM_CHANNELS;
	output_parameters.sampleFormat =  paInt16;
	output_parameters.suggestedLatency = Pa_GetDeviceInfo(output_parameters.device)->defaultLowOutputLatency;
	output_parameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(&output_stream,		// the output stream
						NULL, 				// input params
						&output_parameters,	// output params
						SAMPLE_RATE,		// sample rate
						FRAMES_PER_BUFFER,	// frames per buffer
						paClipOff,      	// we won't output out of range samples so don't bother clipping them
						paOutputCallback,	// PortAudio callback function
						&data);				// data pointer
	if(err != paNoError) {
		logger.error("Failed to open output stream: %s", Pa_GetErrorText(err));
		exit(-1);
	}

	err = Pa_StartStream(output_stream);
	if(err != paNoError) {
		logger.error("Failed to start output stream: %s", Pa_GetErrorText(err));
		exit(-1);
	}

	///////////////////////
	// init mumble library
	///////////////////////
	// open output audio stream, pipe incoming audio PCM data to output audio stream

	// This stuff should be on a separate thread
	MumpiCallback mumble_callback(data.out_buf);
	mumlib::MumlibConfiguration conf;
	conf.opusEncoderBitrate = SAMPLE_RATE;
	mumlib::Mumlib mum(mumble_callback, conf);
	mumble_callback.mum = &mum;
	std::thread mumble_thread([&]() {
		while(!sig_caught) {
			try {
				logger.info("Connecting to %s", server.c_str());
				mum.connect(server, 64738, username, "");
				mum.run();
			} catch (mumlib::TransportException &exp) {
				logger.error("TransportException: %s.", exp.what());
				logger.error("Attempting to reconnect in 5 s.");
				std::this_thread::sleep_for(std::chrono::seconds(5));
			}
		}
	});

	std::thread input_consumer_thread([&]() {
		// consumes the data that the input audio thread receives and sends it
		// through mumble client
		// this will continuously read from the input data circular buffer

		// Opus can encode frames of 2.5, 5, 10, 20, 40, or 60 ms
		// the Opus RFC 6716 recommends using 20ms frame sizes
		// so at 48k sample rate, 20ms is 960 samples
		const int OPUS_FRAME_SIZE = 960;
		int16_t *outBuf = new int16_t[MAX_SAMPLES];
		while(!sig_caught) {
			if(!data.rec_buf->isEmpty() && data.rec_buf->getRemaining() >= OPUS_FRAME_SIZE) {
				// do a bulk get and send it through mumble client
				if(mum.getConnectionState() == mumlib::ConnectionState::CONNECTED) {
					const size_t samplesRetrieved = data.rec_buf->top(outBuf, 0, OPUS_FRAME_SIZE);
					mum.sendAudioData(outBuf, OPUS_FRAME_SIZE);
				}
			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		}
		delete[] outBuf;
	});

	// init signal handler
	struct sigaction action;
	action.sa_handler = sigHandler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);

	// busy loop until signal is caught
	while(!sig_caught) {
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}

	///////////////////////
	// CLEAN UP
	///////////////////////
	logger.info("Cleaning up...");

	///////////////////////////
	// clean up mumble library
	///////////////////////////
	logger.info("Disconnecting...");
	input_consumer_thread.join();
	mum.disconnect();
	mumble_thread.join();

	///////////////////////////
	// clean up audio library
	///////////////////////////
	logger.info("Cleaning up PortAudio...");

	// close streams
	err = Pa_CloseStream(input_stream);
	if(err != paNoError) {
		logger.error("Failed to close inputstream: %s", Pa_GetErrorText(err));
		exit(-1);
	}
	err = Pa_CloseStream(output_stream);
	if(err != paNoError) {
		logger.error("Failed to close output stream: %s", Pa_GetErrorText(err));
		exit(-1);
	}

	// terminate PortAudio engine
	err = Pa_Terminate();
	if(err != paNoError) {
		logger.error("PortAudio error: %s", Pa_GetErrorText(err));
		exit(-1);
	}
	return 0;
}
