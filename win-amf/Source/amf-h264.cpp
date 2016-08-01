/*
MIT License

Copyright (c) 2016 Michael Fabian Dirks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include "amf-h264.h"

AMFEncoder::VCE::VCE(H264_Encoder_Type encoderType) {
	AMF_RESULT res;
	VCE_Capabilities::EncoderCaps* encoderCaps;

	try {
		// Set Encoder Type
		m_encoderType = encoderType;

		// Create AMF Context
		res = AMFCreateContext(&m_AMFContext);
		if (res != AMF_OK) {
			std::vector<char> msgBuf(1024);
			tempFormatAMFError(&msgBuf, "<AMFEncoder::VCE::H264> AMFCreateContext failed, error %s (code %d).", res);
			AMF_LOG_ERROR("%s", msgBuf.data());
			throw std::exception(msgBuf.data());
		}

		// Create AMF VCE Component depending on Type.
		switch (m_encoderType) {
			case H264_ENCODER_TYPE_AVC:
				AMF_LOG_INFO("<AMFEncoder::VCE::H264> Attempting to create AVC Encoder...");
				encoderCaps = &(VCE_Capabilities::getInstance()->m_AVCCaps);
				if (encoderCaps->acceleration_type != amf::AMF_ACCEL_HARDWARE) {
					AMF_LOG_WARNING("<AMFEncoder::VCE::H264> AVC Encoder is not Hardware-Accelerated!");
				}

				res = AMFCreateComponent(m_AMFContext, AMFVideoEncoderVCE_AVC, &m_AMFEncoder);
				break;
			case H264_ENCODER_TYPE_SVC:
				AMF_LOG_INFO("<AMFEncoder::VCE::H264> Attempting to create SVC Encoder...");
				encoderCaps = &(VCE_Capabilities::getInstance()->m_SVCCaps);
				if (encoderCaps->acceleration_type != amf::AMF_ACCEL_HARDWARE) {
					AMF_LOG_WARNING("<AMFEncoder::VCE::H264> SVC Encoder is not Hardware-Accelerated!");
				}

				res = AMFCreateComponent(m_AMFContext, AMFVideoEncoderVCE_SVC, &m_AMFEncoder);
				break;
		}
		if (res != AMF_OK) {
			std::vector<char> msgBuf(1024);
			tempFormatAMFError(&msgBuf, "<AMFEncoder::VCE::H264> AMFCreateComponent failed, error %s (code %d).", res);
			AMF_LOG_ERROR("%s", msgBuf.data());
			throw std::exception(msgBuf.data());
		}
		//////////////////////////////////////////////////////////////////////////
		// Set static Properties (Can only do this before StartUp())
		//////////////////////////////////////////////////////////////////////////
		/// Usage
		switch (Usage) {
			case H264_USAGE_WEBCAM:
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_WEBCAM);
				AMF_LOG_INFO("<AMFEncoder::VCE::H264> Usage: Web-Camera");
				break;
			case H264_USAGE_ULTRA_LOW_LATENCY:
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
				AMF_LOG_INFO("<AMFEncoder::VCE::H264> Usage: Ultra Low Latency");
				break;
			case H264_USAGE_LOW_LATENCY:
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
				AMF_LOG_INFO("<AMFEncoder::VCE::H264> Usage: Low Latency");
				break;
			case H264_USAGE_TRANSCODING:
			default:
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
				AMF_LOG_INFO("<AMFEncoder::VCE::H264> Usage: Transcoding");
				break;
		}
		if (res != AMF_OK) {
			std::vector<char> msgBuf(1024);
			tempFormatAMFError(&msgBuf, "<AMFEncoder::VCE::H264> SetProperty(AMF_VIDEO_ENCODER_USAGE) failed, error %s (code %d).", res);
			AMF_LOG_ERROR("%s", msgBuf.data());
			throw std::exception(msgBuf.data());
		}

		/// Quality Preset
		switch (QualityPreset) {
			case H264_QUALITY_PRESET_SPEED:
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
				AMF_LOG_INFO("<AMFEncoder::VCE::H264> Quality Preset: Speed");
				break;
			case H264_QUALITY_PRESET_BALANCED:
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED);
				AMF_LOG_INFO("<AMFEncoder::VCE::H264> Quality Preset: Balanced");
				break;
			case H264_QUALITY_PRESET_QUALITY:
			default:
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY);
				AMF_LOG_INFO("<AMFEncoder::VCE::H264> Quality Preset: Quality");
				break;
		}
		if (res != AMF_OK) {
			std::vector<char> msgBuf(1024);
			tempFormatAMFError(&msgBuf, "<AMFEncoder::VCE::H264> SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET) failed, error %s (code %d).", res);
			AMF_LOG_ERROR("%s", msgBuf.data());
			throw std::exception(msgBuf.data());
		}

		/// Frame Size
		if (((framesize.first >= encoderCaps->input.minWidth) && (framesize.first <= encoderCaps->input.maxWidth))
			&& ((framesize.second >= encoderCaps->input.minHeight) && (framesize.second <= encoderCaps->input.maxHeight))) {

		}

	} catch(...) {
		if (m_AMFEncoder)
			m_AMFEncoder->Terminate();
		if (m_AMFContext)
			m_AMFContext->Terminate();

		throw;
	}
}

AMFEncoder::VCE::~VCE() {
	if (m_AMFEncoder)
		m_AMFEncoder->Terminate();
	if (m_AMFContext)
		m_AMFContext->Terminate();
}

void AMFEncoder::VCE::SetMemoryType(H264_Memory_Type memoryType) {
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetMemoryType> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	m_memoryType = memoryType;

	char* memoryTypes[] = {
		"Host",
		"DirectX11",
		"OpenGL"
	};
	AMF_LOG_INFO("<AMFEncoder::VCE::SetMemoryType> Set to %s.", memoryTypes[m_memoryType]);
}

AMFEncoder::H264_Memory_Type AMFEncoder::VCE::GetMemoryType() {
	return m_memoryType;
}

void AMFEncoder::VCE::SetSurfaceFormat(H264_Surface_Format surfaceFormat) {
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetSurfaceFormat> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	m_surfaceFormat = surfaceFormat;

	char* surfaceFormats[] = {
		"NV12",
		"I420",
		"I444",
		"RGB"
	};
	AMF_LOG_INFO("<AMFEncoder::VCE::SetSurfaceFormat> Set to %s.", surfaceFormats[m_surfaceFormat]);
}

AMFEncoder::H264_Surface_Format AMFEncoder::VCE::GetSurfaceFormat() {
	return m_surfaceFormat;
}

void AMFEncoder::VCE::SetUsage(H264_Usage usage) {
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetUsage> Attempted to change Surface Format while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	m_surfaceFormat = surfaceFormat;

	char* usages[] = {
		"NV12",
		"I420",
		"I444",
		"RGB"
	};
	AMF_LOG_INFO("<AMFEncoder::VCE::SetUsage> Surface Format set to %s.", surfaceFormats[m_surfaceFormat]);
}

void AMFEncoder::VCE::SetQualityPreset(H264_Quality_Preset qualityPreset) {

}

void AMFEncoder::VCE::SetProfile(H264_Profile profile) {

}

void AMFEncoder::VCE::SetProfileLevel(H264_Profile_Level profileLevel) {

}

void AMFEncoder::VCE::SetMaxOfLTRFrames(uint32_t maxOfLTRFrames) {

}

void AMFEncoder::VCE::SetScanType(H264_ScanType scanType) {

}

void AMFEncoder::VCE::SetFrameSize(std::pair<uint32_t, uint32_t>& framesize) {

}

void AMFEncoder::VCE::SetFrameRate(std::pair<uint32_t, uint32_t>& framerate) {

}

void AMFEncoder::VCE::tempFormatAMFError(std::vector<char>* buffer, const char* format, AMF_RESULT res) {
	std::vector<char> errBuf(1024);
	wcstombs(errBuf.data(), amf::AMFGetResultText(res), errBuf.size());
	sprintf(buffer->data(), format, errBuf, res);
}
