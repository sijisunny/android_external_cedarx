#ifndef CEDARX_RECORDER_H_
#define CEDARX_RECORDER_H_

#include <media/MediaRecorderBase.h>
#include <camera/CameraParameters.h>
#include <utils/String8.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <media/AudioRecord.h>
#include <camera/ICameraRecordingProxyListener.h>

namespace android {

class Camera;
class AudioRecord;

#define AUDIO_LATENCY_TIME	700000		// US
#define VIDEO_LATENCY_TIME	700000		// US
#define MAX_FILE_SIZE		(2*1024*1024*1024 - 64*1024)

class CedarXRecorder{
public:
    CedarXRecorder();
    ~CedarXRecorder();
	
    status_t setCamera(const sp<ICamera>& camera, const sp<ICameraRecordingProxy>& proxy);
	status_t setParamVideoCameraId(int32_t cameraId);
    status_t setListener(const sp<IMediaRecorderClient>& listener);
    status_t setPreviewSurface(const sp<Surface>& surface);
    status_t prepare();
    status_t start();
    status_t pause();
    status_t stop();
    status_t reset();
    status_t getMaxAmplitude(int *max);
	void releaseCamera();

	// Encoding parameter handling utilities

	// audio
	status_t setAudioSource(audio_source_t as);
    status_t setAudioEncoder(audio_encoder ae);
	status_t setParamAudioEncodingBitRate(int32_t bitRate);
    status_t setParamAudioNumberOfChannels(int32_t channles);
    status_t setParamAudioSamplingRate(int32_t sampleRate);

	// video
	status_t setVideoSource(video_source vs);
    status_t setVideoEncoder(video_encoder ve);
	status_t setParamVideoEncodingBitRate(int32_t bitRate);
    status_t setVideoSize(int width, int height);
    status_t setVideoFrameRate(int frames_per_second);
	status_t setParamVideoRotation(int32_t degrees);

	// output
    status_t setOutputFormat(output_format of);
	status_t setParamMaxFileDurationUs(int64_t timeUs);
	status_t setParamMaxFileSizeBytes(int64_t bytes);
    status_t setOutputFile(int fd);

	void CedarXReleaseFrame(int index);
	void dataCallbackTimestamp(int64_t timestampUs, int32_t msgType, const sp<IMemory> &data);

	status_t CedarXReadAudioBuffer(void *pbuf, int *size, int64_t *timeStamp);

	status_t setParamTimeLapseEnable(int32_t timeLapseEnable);
    status_t setParamTimeBetweenTimeLapseFrameCapture(int64_t timeUs);

	int CedarXRecorderCallback(int event, void *info);

	class CameraProxyListener: public BnCameraRecordingProxyListener {
	public:
		CameraProxyListener(CedarXRecorder* recorder);
		virtual void dataCallbackTimestamp(int64_t timestampUs, int32_t msgType,
				const sp<IMemory> &data);

	private:
		CedarXRecorder * mRecorder;
	};

	// isBinderAlive needs linkToDeath to work.
	class DeathNotifier: public IBinder::DeathRecipient {
	public:
		DeathNotifier() {}
		virtual void binderDied(const wp<IBinder>& who)
		{
			LOGI("Camera recording proxy died");
		}
	};

private:
	
	status_t CreateAudioRecorder();
	void releaseOneRecordingFrame(const sp<IMemory>& frame);
	status_t isCameraAvailable(const sp<ICamera>& camera,
								   const sp<ICameraRecordingProxy>& proxy,
								   int32_t cameraId);
	audio_source_t mAudioSource;
	video_source mVideoSource;

	enum {
        kMaxBufferSize = 2048,

        // After the initial mute, we raise the volume linearly
        // over kAutoRampDurationUs.
        kAutoRampDurationUs = 300000,

        // This is the initial mute duration to suppress
        // the video recording signal tone
        kAutoRampStartUs = 700000,
    };

    sp<Camera> mCamera;
	sp<ICameraRecordingProxy> mCameraProxy;
    sp<Surface> mPreviewSurface;
    sp<IMediaRecorderClient> mListener;

    output_format mOutputFormat;
    audio_encoder mAudioEncoder;
    video_encoder mVideoEncoder;
	
    int32_t mCameraId;
    int32_t mVideoWidth, mVideoHeight;
    int32_t mFrameRate;
    int32_t mVideoBitRate;
	int64_t mMaxFileDurationUs;
	int64_t mMaxFileSizeBytes;
	int32_t mRotationDegrees;  // Clockwise

    int32_t mAudioBitRate;
    int32_t mAudioChannels;
	int32_t mSampleRate;

	bool mCaptureTimeLapse;
	// Time between capture of two frames during time lapse recording
    // Negative value indicates that timelapse is disabled.
	int64_t mTimeBetweenTimeLapseFrameCaptureUs;
	// Time between two frames in final video (1/frameRate)
    int64_t mTimeBetweenTimeLapseVideoFramesUs;
	// Real timestamp of the last encoded time lapse frame
    int64_t mLastTimeLapseFrameRealTimestampUs;
	int64_t mLastTimeLapseFrameTimestampUs;

    int mOutputFd;
	int32_t mCameraFlags;

    enum CameraFlags {
        FLAGS_SET_CAMERA = 1L << 0,
        FLAGS_HOT_CAMERA = 1L << 1,
    };

	Mutex mStateLock;
	
	sp<MemoryHeapBase>  mFrameHeap;
	sp<MemoryBase>      mFrameBuffer;

	bool				mStarted;
	uint 				mRecModeFlag;
	AudioRecord 		* mRecord;

	int64_t				mLatencyStartUs;

	sp<DeathNotifier> mDeathNotifier;
	
    CedarXRecorder(const CedarXRecorder &);
    CedarXRecorder &operator=(const CedarXRecorder &);
};

}  // namespace android

#endif  // CEDARX_RECORDER_H_

