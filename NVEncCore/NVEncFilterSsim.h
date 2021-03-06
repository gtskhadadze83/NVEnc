﻿// -----------------------------------------------------------------------------------------
// NVEnc by rigaya
// -----------------------------------------------------------------------------------------
//
// The MIT License
//
// Copyright (c) 2019 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

#pragma once

#include <array>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
#include "NVEncFilter.h"
#include "NVEncParam.h"

#define ENABLE_SSIM (ENABLE_AVSW_READER)

#if ENABLE_SSIM

class CuvidDecode;

class NVEncFilterParamSsim : public NVEncFilterParam {
public:
    bool ssim;
    bool psnr;
    int deviceId;
    CUvideoctxlock vidctxlock;
    VideoInfo input;
    rgy_rational<int> streamtimebase;

    NVEncFilterParamSsim() : ssim(true), psnr(false), deviceId(0), vidctxlock(), input(), streamtimebase() {

    };
    virtual ~NVEncFilterParamSsim() {};
    virtual tstring print() const override;
};

class NVEncFilterSsim : public NVEncFilter {
public:
    NVEncFilterSsim();
    virtual ~NVEncFilterSsim();
    virtual RGY_ERR init(shared_ptr<NVEncFilterParam> pParam, shared_ptr<RGYLog> pPrintMes) override;
    RGY_ERR initDecode(const RGYBitstream *bitstream);
    bool decodeStarted() { return m_decodeStarted; }
    RGY_ERR thread_func();
    RGY_ERR compare_frames(bool flush);

    RGY_ERR addBitstream(const RGYBitstream *bitstream);
    virtual void showResult();
protected:
    RGY_ERR init_cuda_resources();
    void close_cuda_resources();
    virtual RGY_ERR run_filter(const FrameInfo *pInputFrame, FrameInfo **ppOutputFrames, int *pOutputFrameNum, cudaStream_t stream) override;
    virtual void close() override;
    virtual RGY_ERR calc_ssim_psnr(const FrameInfo *p0, const FrameInfo *p1);

    bool m_decodeStarted; //デコードが開始したか
    int m_deviceId;       //SSIM計算で使用するCUDA device ID

    //スレッド関連
    std::thread m_thread; //スレッド本体
    std::mutex m_mtx;     //m_input, m_unused操作用のロック
    bool m_abort;         //スレッド中断用

    CUvideoctxlock m_vidctxlock; //cuvid用のlock
    std::deque<std::unique_ptr<CUFrameBuf>> m_input;  //使用中のフレームバッファ(オリジナルフレーム格納用)
    std::deque<std::unique_ptr<CUFrameBuf>> m_unused; //使っていないフレームバッファ(オリジナルフレーム格納用)
    std::unique_ptr<CuvidDecode> m_decoder;     // デコーダエンジン
    unique_ptr<NVEncFilterCspCrop> m_crop;      // NV12->YV12変換用
    std::unique_ptr<CUFrameBuf> m_decFrameCopy; //デコード後にcrop(NV12->YV12変換)したフレームの格納場所
    std::array<CUMemBufPair, 3> m_tmpSsim; //評価結果を返すための一時バッファ
    std::array<CUMemBufPair, 3> m_tmpPsnr; //評価結果を返すための一時バッファ
    unique_ptr<cudaEvent_t, cudaevent_deleter> m_cropEvent; //デコードしたフレームがcrop(NV12->YV12変換)し終わったかを示すイベント
    std::unique_ptr<cudaStream_t, cudastream_deleter> m_streamCrop; //デコードしたフレームをcrop(NV12->YV12変換)するstream
    std::array<std::unique_ptr<cudaStream_t, cudastream_deleter>, 3> m_streamCalcSsim; //評価計算を行うstream
    std::array<std::unique_ptr<cudaStream_t, cudastream_deleter>, 3> m_streamCalcPsnr; //評価計算を行うstream
    std::array<double, 3> m_planeCoef;      // 評価結果に関する YUVの重み
    std::array<double, 3> m_ssimTotalPlane; // 評価結果の累積値 YUV
    double m_ssimTotal;                     // 評価結果の累積値 All
    std::array<double, 3> m_psnrTotalPlane; // 評価結果の累積値 YUV
    double m_psnrTotal;                     // 評価結果の累積値 All
    int m_frames;                           // 評価したフレーム数
};

#endif //#if ENABLE_SSIM
