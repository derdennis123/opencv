// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2017, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.

#include "../precomp.hpp"

#ifdef HAVE_PROTOBUF
#include <fstream>
#include "caffe_io.hpp"
#endif

namespace cv { namespace dnn {
CV__DNN_EXPERIMENTAL_NS_BEGIN

#ifdef HAVE_PROTOBUF

void shrinkCaffeModel(const String& src, const String& dst)
{
    CV_TRACE_FUNCTION();

    caffe::NetParameter net;
    ReadNetParamsFromBinaryFileOrDie(src.c_str(), &net);

    for (int i = 0; i < net.layer_size(); ++i)
    {
        caffe::LayerParameter* lp = net.mutable_layer(i);
        for (int j = 0; j < lp->blobs_size(); ++j)
        {
            caffe::BlobProto* blob = lp->mutable_blobs(j);
            CV_Assert(blob->data_size() != 0);  // float32 array.

            Mat floats(1, blob->data_size(), CV_32FC1, (void*)blob->data().data());
            Mat halfs(1, blob->data_size(), CV_16SC1);
            convertFp16(floats, halfs);  // Convert to float16.

            blob->clear_data();  // Clear float32 data.

            // Set float16 data.
            blob->set_raw_data(halfs.data, halfs.total() * halfs.elemSize());
            blob->set_raw_data_type(caffe::FLOAT16);
        }
    }
    size_t msgSize = net.ByteSizeLong();
    std::vector<uint8_t> output(msgSize);
    net.SerializeWithCachedSizesToArray(&output[0]);

    std::ofstream ofs(dst.c_str(), std::ios::binary);
    ofs.write((const char*)&output[0], msgSize);
    ofs.close();
}

#else

void shrinkCaffeModel(const String& src, const String& dst)
{
    CV_Error(cv::Error::StsNotImplemented, "libprotobuf required to import data from Caffe models");
}

#endif  // HAVE_PROTOBUF

CV__DNN_EXPERIMENTAL_NS_END
}} // namespace
