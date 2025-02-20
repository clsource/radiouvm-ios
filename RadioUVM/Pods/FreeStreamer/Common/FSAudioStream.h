/*
 * This file is part of the FreeStreamer project,
 * (C)Copyright 2011-2014 Matias Muhonen <mmu@iki.fi>
 * See the file ''LICENSE'' for using the code.
 *
 * https://github.com/muhku/FreeStreamer
 */

#import <Foundation/Foundation.h>

/**
 * The major version of the current release.
 */
#define FREESTREAMER_VERSION_MAJOR          2

/**
 * The minor version of the current release.
 */
#define FREESTREAMER_VERSION_MINOR          8

/**
 * The reversion of the current release
 */
#define FREESTREAMER_VERSION_REVISION       0

/**
 * Follow this notification for the audio stream state changes.
 */
extern NSString* const FSAudioStreamStateChangeNotification;
extern NSString* const FSAudioStreamNotificationKey_State;

/**
 * Follow this notification for the audio stream errors.
 */
extern NSString* const FSAudioStreamErrorNotification;
extern NSString* const FSAudioStreamNotificationKey_Error;

/**
 * Follow this notification for the audio stream metadata.
 */
extern NSString* const FSAudioStreamMetaDataNotification;
extern NSString* const FSAudioStreamNotificationKey_MetaData;

/**
 * The audio stream state.
 */
typedef enum {
    kFsAudioStreamRetrievingURL,
    kFsAudioStreamStopped,
    kFsAudioStreamBuffering,
    kFsAudioStreamPlaying,
    kFsAudioStreamPaused,
    kFsAudioStreamSeeking,
    kFSAudioStreamEndOfFile,
    kFsAudioStreamFailed,
    kFsAudioStreamUnknownState
} FSAudioStreamState;

/**
 * The audio stream errors.
 */
typedef enum {
    kFsAudioStreamErrorNone = 0,
    kFsAudioStreamErrorOpen = 1,
    kFsAudioStreamErrorStreamParse = 2,
    kFsAudioStreamErrorNetwork = 3,
    kFsAudioStreamErrorUnsupportedFormat = 4,
    kFsAudioStreamErrorStreamBouncing = 5
} FSAudioStreamError;

@protocol FSPCMAudioStreamDelegate;
@class FSAudioStreamPrivate;

/**
 * The audio stream playback position.
 */
typedef struct {
    unsigned minute;
    unsigned second;
} FSStreamPosition;

/**
 * The audio stream seek byte offset.
 */
typedef struct {
    UInt64 start;
    UInt64 end;
    unsigned position;
} FSSeekByteOffset;

/**
 * The low-level stream configuration.
 */
@interface FSStreamConfiguration : NSObject {
}

/**
 * The number of buffers.
 */
@property (nonatomic,assign) unsigned bufferCount;
/**
 * The size of each buffer.
 */
@property (nonatomic,assign) unsigned bufferSize;
/**
 * The number of packet descriptions.
 */
@property (nonatomic,assign) unsigned maxPacketDescs;
/**
 * The decode queue size.
 */
@property (nonatomic,assign) unsigned decodeQueueSize;
/**
 * The HTTP connection buffer size.
 */
@property (nonatomic,assign) unsigned httpConnectionBufferSize;
/**
 * The output sample rate.
 */
@property (nonatomic,assign) double   outputSampleRate;
/**
 * The number of output channels.
 */
@property (nonatomic,assign) long     outputNumChannels;
/**
 * The interval within the stream may enter to the buffering state before it fails.
 */
@property (nonatomic,assign) int      bounceInterval;
/**
 * The number of times the stream may enter the buffering state before it fails.
 */
@property (nonatomic,assign) int      maxBounceCount;
/**
 * The stream must start within this seconds before it fails.
 */
@property (nonatomic,assign) int      startupWatchdogPeriod;
/**
 * Allow buffering of this many bytes before the cache is full.
 */
@property (nonatomic,assign) int      maxPrebufferedByteCount;
/**
 * The HTTP user agent used for stream operations.
 */
@property (nonatomic,strong) NSString *userAgent;
/**
 * The directory used for caching the streamed files.
 */
@property (nonatomic,strong) NSString *cacheDirectory;
/**
 * The property determining if caching the streams to the disk is enabled.
 */
@property (nonatomic,assign) BOOL cacheEnabled;
/**
 * The maximum size of the disk cache in bytes.
 */
@property (nonatomic,assign) int maxDiskCacheSize;

@end

NSString*             freeStreamerReleaseVersion();

/**
 * FSAudioStream is a class for streaming audio files from an URL.
 * It must be directly fed with an URL, which contains audio. That is,
 * playlists or other non-audio formats yield an error.
 *
 * To start playback, the stream must be either initialized with an URL
 * or the playback URL can be set with the url property. The playback
 * is started with the play method. It is possible to pause or stop
 * the stream with the respective methods.
 *
 * Non-continuous streams (audio streams with a known duration) can be
 * seeked with the seekToPosition method.
 *
 * Note that FSAudioStream is not designed to be thread-safe! That means
 * that using the streamer from multiple threads without syncronization
 * could cause problems. It is recommended to keep the streamer in the
 * main thread and call the streamer methods only from the main thread
 * (consider using performSelectorOnMainThread: if calls from multiple
 * threads are needed).
 */
@interface FSAudioStream : NSObject {
    FSAudioStreamPrivate *_private;
}

/**
 * Initializes the audio stream with an URL.
 *
 * @param url The URL from which the stream data is retrieved.
 */
- (id)initWithUrl:(NSURL *)url;

/**
 * Initializes the stream with a configuration.
 *
 * @param configuration The stream configuration.
 */
- (id)initWithConfiguration:(FSStreamConfiguration *)configuration;

/**
 * Starts playing the stream. If no playback URL is
 * defined, an error will occur.
 */
- (void)play;

/**
 * Starts playing the stream from the given URL.
 *
 * @param url The URL from which the stream data is retrieved.
 */
- (void)playFromURL:(NSURL*)url;

/**
 * Starts playing the stream from the given offset.
 * The offset can be retrieved from the stream with the
 * currentSeekByteOffset property.
 *
 * @param offset The offset where to start playback from.
 */
- (void)playFromOffset:(FSSeekByteOffset)offset;

/**
 * Stops the stream playback.
 */
- (void)stop;

/**
 * If the stream is playing, the stream playback is paused upon calling pause.
 * Otherwise (the stream is paused), calling pause will continue the playback.
 */
- (void)pause;

/**
 * Seeks the stream to a given position. Requires a non-continuous stream
 * (a stream with a known duration).
 *
 * @param position The stream position to seek to.
 */
- (void)seekToPosition:(FSStreamPosition)position;

/**
 * Sets the audio stream volume from 0.0 to 1.0.
 * Note that the overall volume is still constrained by the volume
 * set by the user! So the actual volume cannot be higher
 * than the volume currently set by the user. For example, if
 * requesting a volume of 0.5, then the volume will be 50%
 * lower than the current playback volume set by the user.
 *
 * @param volume The audio stream volume.
 */
- (void)setVolume:(float)volume;

/**
 * Sets the audio stream playback rate from 0.5 to 2.0.
 * Value 1.0 means the normal playback rate. Values below
 * 1.0 means a slower playback rate than usual and above
 * 1.0 a faster playback rate. Notice that using a faster
 * playback rate than 1.0 may mean that you have to increase
 * the buffer sizes for the stream still to play.
 *
 * The play rate has only effect if the stream is playing.
 *
 * @param playRate The playback rate.
 */
- (void)setPlayRate:(float)playRate;

/**
 * Returns the playback status: YES if the stream is playing, NO otherwise.
 */
- (BOOL)isPlaying;

/**
 * The stream URL.
 */
@property (nonatomic,assign) NSURL *url;
/**
 * Determines if strict content type checking  is required. If the audio stream
 * cannot determine that the stream is actually an audio stream, the stream
 * does not play. Disabling strict content type checking bypasses the
 * stream content type checks and tries to play the stream regardless
 * of the content type information given by the server.
 */
@property (nonatomic,assign) BOOL strictContentTypeChecking;
/**
 * Set an output file to store the stream contents to a file.
 */
@property (nonatomic,assign) NSURL *outputFile;
/**
 * Sets a default content type for the stream. Only used when strict content
 * type checking is disabled.
 */
@property (nonatomic,assign) NSString *defaultContentType;
/**
 * The property has the content type of the stream, for instance audio/mpeg.
 */
@property (nonatomic,readonly) NSString *contentType;
/**
 * The property has the suggested file extension for the stream based on the stream content type.
 */
@property (nonatomic,readonly) NSString *suggestedFileExtension;
/**
 * This property has the current playback position, if the stream is non-continuous.
 * The current playback position cannot be determined for continuous streams.
 */
@property (nonatomic,readonly) FSStreamPosition currentTimePlayed;
/**
 * This property has the duration of the stream, if the stream is non-continuous.
 * Continuous streams do not have a duration.
 */
@property (nonatomic,readonly) FSStreamPosition duration;
/**
 * This property has the current seek byte offset of the stream, if the stream is non-continuous.
 * Continuous streams do not have a seek byte offset.
 */
@property (nonatomic,readonly) FSSeekByteOffset currentSeekByteOffset;
/**
 * The property is true if the stream is continuous (no known duration).
 */
@property (nonatomic,readonly) BOOL continuous;
/**
 * The property is true if the stream has been cached locally.
 */
@property (nonatomic,readonly) BOOL cached;
/**
 * This property has the number of bytes buffered for this stream.
 */
@property (nonatomic,readonly) size_t prebufferedByteCount;
/**
 * Called upon completion of the stream. Note that for continuous
 * streams this is never called.
 */
@property (copy) void (^onCompletion)();
/**
 * Called upon a state change.
 */
@property (copy) void (^onStateChange)(FSAudioStreamState state);
/**
 * Called upon a meta data is available.
 */
@property (copy) void (^onMetaDataAvailable)(NSDictionary *metadata);
/**
 * Called upon a failure.
 */
@property (copy) void (^onFailure)(FSAudioStreamError error);
/**
 * The property has the low-level stream configuration.
 */
@property (readonly) FSStreamConfiguration *configuration;
/**
 * Delegate.
 */
@property (nonatomic,unsafe_unretained) IBOutlet id<FSPCMAudioStreamDelegate> delegate;

@end

/**
 * To access the PCM audio data, use this delegate.
 */
@protocol FSPCMAudioStreamDelegate <NSObject>

@optional
/**
 * Called when there are PCM audio samples available. Do not do any blocking operations
 * when you receive the data. Instead, copy the data and process it so that the
 * main event loop doesn't block. Failing to do so may cause glitches to the audio playback.
 *
 * @param audioStream The audio stream the samples are from.
 * @param samples The PCM audio samples.
 * @param count The number of samples available.
 */
- (void)audioStream:(FSAudioStream *)audioStream samplesAvailable:(const int16_t *)samples count:(NSUInteger)count;
@end
