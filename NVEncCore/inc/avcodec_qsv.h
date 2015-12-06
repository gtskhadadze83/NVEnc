﻿//  -----------------------------------------------------------------------------------------
//    NVEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  ---------------------------------------------------------------------------------------
#ifndef _AVCODEC_QSV_H_
#define _AVCODEC_QSV_H_

#include "NVEncVersion.h"

#if ENABLE_AVCUVID_READER
#include <algorithm>

#pragma warning (push)
#pragma warning (disable: 4244)
extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}
#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "swresample.lib")
#pragma warning (pop)

#include "nvcuvid.h"
#include "NVEncUtil.h"

#if _DEBUG
#define NV_AV_LOG_LEVEL AV_LOG_WARNING
#else
#define NV_AV_LOG_LEVEL AV_LOG_ERROR
#endif

typedef struct CuvidCodec {
    uint32_t codec_id;   //avcodecのコーデックID
    cudaVideoCodec cuvid_cc; //QSVのfourcc
} CuvidCodec;

//QSVでデコード可能なコーデックのリスト
static const CuvidCodec CUVID_DECODE_LIST[] = { 
    { AV_CODEC_ID_H264,       cudaVideoCodec_H264   },
    //{ AV_CODEC_ID_HEVC,       cudaVideoCodec_HEVC  },
    { AV_CODEC_ID_MPEG1VIDEO, cudaVideoCodec_MPEG1 },
    { AV_CODEC_ID_MPEG2VIDEO, cudaVideoCodec_MPEG2 },
    //{ AV_CODEC_ID_VC1,        cudaVideoCodec_VC1   },
    //{ AV_CODEC_ID_WMV3,       cudaVideoCodec_VC1   },
    //{ AV_CODEC_ID_MPEG4,      cudaVideoCodec_MPEG4   },
};

static tstring CodecIdToStr(cudaVideoCodec cuvid_cc) {
    switch (cuvid_cc) {
    case cudaVideoCodec_H264:  return _T("H.264/AVC");
    case cudaVideoCodec_HEVC:  return _T("H.265/HEVC");
    case cudaVideoCodec_MPEG2: return _T("MPEG2");
    case cudaVideoCodec_MPEG1: return _T("MPEG1");
    case cudaVideoCodec_VC1:   return _T("VC-1");
    case cudaVideoCodec_MPEG4: return _T("MPEG4");
    case cudaVideoCodec_VP8:   return _T("VP8");
    case cudaVideoCodec_VP9:   return _T("VP9");
    default: return _T("unknown");
    }
}

static const TCHAR *AVQSV_CODEC_AUTO = _T("auto");
static const TCHAR *AVQSV_CODEC_COPY = _T("copy");

static const int AVQSV_DEFAULT_AUDIO_BITRATE = 192;

static inline bool avcodecIsCopy(const TCHAR *codec) {
    return codec == nullptr || 0 == _tcsicmp(codec, AVQSV_CODEC_COPY);
}
static inline bool avcodecIsAuto(const TCHAR *codec) {
    return codec != nullptr && 0 == _tcsicmp(codec, AVQSV_CODEC_AUTO);
}

static const AVRational CUVID_NATIVE_TIMEBASE = { 1, 10000000 };
static const TCHAR *AVCODEC_DLL_NAME[] = {
    _T("avcodec-57.dll"), _T("avformat-57.dll"), _T("avutil-55.dll"), _T("swresample-2.dll")
};

enum AVQSVCodecType : uint32_t {
    AVQSV_CODEC_DEC = 0x01,
    AVQSV_CODEC_ENC = 0x02,
};

enum AVQSVFormatType : uint32_t {
    AVQSV_FORMAT_DEMUX = 0x01,
    AVQSV_FORMAT_MUX   = 0x02,
};

//avcodecのエラーを表示
tstring qsv_av_err2str(int ret);

//コーデックの種類を表示
tstring get_media_type_string(AVCodecID codecId);

//必要なavcodecのdllがそろっているかを確認
bool check_avcodec_dll();

//avcodecのdllが存在しない場合のエラーメッセージ
tstring error_mes_avcodec_dll_not_found();

//avcodecのライセンスがLGPLであるかどうかを確認
bool checkAvcodecLicense();

//avqsvでサポートされている動画コーデックを表示
tstring getAVQSVSupportedCodecList();

//利用可能な音声エンコーダ/デコーダを表示
tstring getAVCodecs(AVQSVCodecType flag);

//利用可能なフォーマットを表示
tstring getAVFormats(AVQSVFormatType flag);

#endif //ENABLE_AVCUVID_READER

#endif //_AVCODEC_QSV_H_
