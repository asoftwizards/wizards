#ifndef _STAMPPDF_H
#define _STAMPPDF_H

#include "acommon/acommon.h"
#include "adata/avalue.h"

#include <qpdf/QPDF.hh> // qpdf-devel-5.1.1-6.el6.i686
#include <qpdf/QUtil.hh> // qpdf-devel-5.1.1-6.el6.i686
#include <qpdf/QPDFWriter.hh> // qpdf-devel-5.1.1-6.el6.i686

namespace Effi {

enum StampPosition { spLeftBottom = 1, spLeftTop, spRightTop, spRightBottom, spCentre, spCustom };

class StampToPDF {
	public :
		StampToPDF();
		StampToPDF(string& fileName);
		StampToPDF(Effi::Blob fileContent);
		~StampToPDF();
		
		void SetColor(int color);
		void SetBackgroundColor(int backgroundColor);
		void SetFont(string& fontFile);
		void SetFontSize(int fontSize);
		void SetPositionStamp(StampPosition sp = spRightBottom);
		void SetPositionStampCustom(int x, int y);
		void SetTextStamp(Effi::Value textStamp);
		
		void Process(string& outFileName);
		Effi::Blob Process();
		
	private :
		StampToPDF(const StampToPDF& spdf);
		StampToPDF& operator= (const StampToPDF& spdf);
		
		void appendStamp();
		
		int width_;
		int height_;
		
		int color_; // Color text stamp, default = 0x000000 (black)
		int backgroundColor_; // Background color stamp, default = 0x7FDDFF
		string fontName_; // Font file text stamp, default = /usr/share/fonts/dejavu/DejaVuSans.ttf
		int fontSizeStatic_; //
		int fontSize_; // Font size text stamp, default = 9
		int stroke_; //
		int radiusCircle_; //
		StampPosition positionStamp_; // Position stamp, default = spRightBottom
		int xPosition_; // Left coordinate stamp
		int yPosition_; // Bottom coordinate stamp
		Effi::Value textStamp_; // Text stamp, string array
		QPDF pdf_;
};

} // namespace Effi

#endif
