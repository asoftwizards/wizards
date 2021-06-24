
#include <sstream>

#include "stamppdf.h"

#include <gd.h> // gd-devel-2.0.35-11.el6.i686

namespace Effi {

const int fontSizeStatic = 8;
const int fontSize = 7;
const int stroke = 3;
const int radiusCircle = 8;
const string fontName("./arial.ttf");
const int color = 0x000000;
const int colorBox = 0x000000;
const int backgroundColor = 0xFFFFFF; // 0x7FDDFF;

StampToPDF::StampToPDF() : width_(612), height_(792), color_(color), backgroundColor_(backgroundColor), fontName_(fontName), fontSizeStatic_(fontSizeStatic), fontSize_(fontSize), stroke_(stroke), radiusCircle_(radiusCircle), positionStamp_(spCentre), xPosition_(0), yPosition_(0) {
	cout << "Empty PDF !!!!!" << endl;
	pdf_.emptyPDF();
	// Create the page dictionary
	QPDFObjectHandle page = pdf_.makeIndirectObject(QPDFObjectHandle::newDictionary());
	page.replaceKey("/Type", QPDFObjectHandle::newName("/Page"));
	QPDFObjectHandle mediabox = QPDFObjectHandle::newArray();
	mediabox.appendItem(QPDFObjectHandle::newInteger(0));
	mediabox.appendItem(QPDFObjectHandle::newInteger(0));
	mediabox.appendItem(QPDFObjectHandle::newInteger(width_));
	mediabox.appendItem(QPDFObjectHandle::newInteger(height_));
	page.replaceKey("/MediaBox", mediabox);
	// Add the page to the PDF file
	pdf_.addPage(page, true);
}

StampToPDF::StampToPDF(string& fileName) : width_(0), height_(0), color_(color), backgroundColor_(backgroundColor), fontName_(fontName), fontSizeStatic_(fontSizeStatic), fontSize_(fontSize), stroke_(stroke), radiusCircle_(radiusCircle), positionStamp_(spCentre), xPosition_(0), yPosition_(0) {
	cout << "PDF from File !!!!!" << endl;
	pdf_.processFile(fileName.c_str());
	// MediaBox
	vector<QPDFObjectHandle> pages = pdf_.getAllPages();
	if ( pages.size() ) {
		for ( size_t ip = 0; ip < pages.size(); ++ip ) {
			QPDFObjectHandle mediabox = pages[ip].getKey("/MediaBox");
			if ( mediabox.isArray() ) {
				vector<QPDFObjectHandle> mbArray = mediabox.getArrayAsVector();
				width_ = (int)mbArray[2].getNumericValue();
				height_ = (int)mbArray[3].getNumericValue();
			}
		}
	}
}

StampToPDF::StampToPDF(Effi::Blob fileContent) : width_(0), height_(0), color_(color), backgroundColor_(backgroundColor), fontName_(fontName), fontSizeStatic_(fontSizeStatic), fontSize_(fontSize), stroke_(stroke), radiusCircle_(radiusCircle), positionStamp_(spCentre), xPosition_(0), yPosition_(0) {
	cout << "PDF from Memory !!!!!" << endl;
	string fileName("memory.pdf");
	pdf_.processMemoryFile(fileName.c_str(), fileContent.Data(), fileContent.Size());
	// MediaBox
	vector<QPDFObjectHandle> pages = pdf_.getAllPages();
	if ( pages.size() ) {
		for ( size_t ip = 0; ip < pages.size(); ++ip ) {
			QPDFObjectHandle mediabox = pages[ip].getKey("/MediaBox");
			if ( mediabox.isArray() ) {
				vector<QPDFObjectHandle> mbArray = mediabox.getArrayAsVector();
				width_ = (int)mbArray[2].getNumericValue();
				height_ = (int)mbArray[3].getNumericValue();
			}
		}
	}
}

StampToPDF::~StampToPDF() {}

void StampToPDF::SetColor(int color) { color_ = color; }
void StampToPDF::SetBackgroundColor(int backgroundColor) { backgroundColor_ = backgroundColor; }
void StampToPDF::SetFont(string& fontFile) { fontName_ = fontFile; }
void StampToPDF::SetFontSize(int fontSize) { fontSize_ = fontSize; }
void StampToPDF::SetPositionStamp(StampPosition sp) { positionStamp_ = sp; }
void StampToPDF::SetPositionStampCustom(int x, int y) { xPosition_ = x; yPosition_ = y; }
void StampToPDF::SetTextStamp(Effi::Value textStamp) { textStamp_ = textStamp.Duplicate(); }

void StampToPDF::Process(string& outFileName) {
	appendStamp();
	QPDFWriter w(pdf_, outFileName.c_str());
	w.write();
}

Effi::Blob StampToPDF::Process() {
	appendStamp();
	QPDFWriter w(pdf_);
	w.setOutputMemory();
	w.write();
	Buffer *pdfBuffer = w.getBuffer();
	return Effi::Blob(pdfBuffer->getSize(), reinterpret_cast<char*>(pdfBuffer->getBuffer()));
}


void StampToPDF::appendStamp() {
	vector<QPDFObjectHandle> pages = pdf_.getAllPages();
 	cout << "pages size = " << pages.size() << endl;
// 	int stroke_ = 3;
// 	int radiusCircle_ = 8;
	if ( pages.size() ) {
 		cout << "width_ = " << width_ << ",   height_ = " << height_ << endl;
 		cout << "textStamp_   static.Size = " << textStamp_["static"].Size() << ",   sign.Size = " << textStamp_["sign"].Size() << endl;
 		cout << "textStamp_ = " << textStamp_ << endl;
		// Bounding Box stamp
		int offsetStatic = 0;
		Effi::Value bbStrings; // Bonding box - ы строк штампа
		int bbWidth = 8 + 2 * stroke_; // Ширина штампа
		int bbHeight = 8 + 2 * stroke_ + 2*(textStamp_["static"].Size()+textStamp_["sign"].Size()-1); // Высота штампа
		{
			// Размеры bounding box штампа и 
			int maxWidthString = 0;
			int brect[8];
			for ( size_t its = 0; its < textStamp_["static"].Size(); ++its ) {
				gdImageStringFT(0, &brect[0], color_, const_cast<char*>(fontName_.c_str()), fontSizeStatic_, 0.0, 0, 0, const_cast<char*>(textStamp_["static"][its].As<string>().c_str()));
				int widthString = brect[2] - brect[6];
				int heightString = brect[3] - brect[7];
				bbHeight += heightString;
				if ( widthString > maxWidthString ) maxWidthString = widthString;
				size_t nBB = bbStrings.Size();
				bbStrings[nBB][0] = widthString;
				bbStrings[nBB][1] = heightString;
			}
// 			offsetStatic = maxWidthString;
			for ( size_t its = 0; its < textStamp_["sign"].Size(); ++its ) {
				gdImageStringFT(0, &brect[0], color_, const_cast<char*>(fontName_.c_str()), fontSize_, 0.0, 0, 0, const_cast<char*>(textStamp_["sign"][its].As<string>().c_str()));
				int widthString = brect[2] - brect[6];
				int heightString = brect[3] - brect[7];
				bbHeight += heightString;
				if ( widthString > maxWidthString ) maxWidthString = widthString;
				size_t nBB = bbStrings.Size();
				bbStrings[nBB][0] = widthString;
				bbStrings[nBB][1] = heightString;
			}
			bbWidth += maxWidthString;
// 			offsetStatic = maxWidthString - offsetStatic;
 			cout << "bbWidth = " << bbWidth << ",  bbHeight = " << bbHeight << endl;
 			cout << "bbStrings = " << bbStrings << endl;
		}
		// Stamp
		gdImagePtr im = gdImageCreateTrueColor(bbWidth, bbHeight);
 		cout << "im :   pointer = " << im << "   pixels = " << im->pixels << "   tpixels = " << im->tpixels
 				<< "   sy = " << im->sy << ",  " << gdImageSY(im) << "   sx = " << im->sx << ",  " << gdImageSX(im) << endl;
		{
			gdImageSetAntiAliased(im, gdTrueColorAlpha(0,0,gdBlueMax,0));
			gdImageFilledRectangle(im, 0, 0, bbWidth, bbHeight, backgroundColor_); // Заливка фона штампа
			gdImageSetThickness(im, stroke_);
			gdImageLine(im, radiusCircle_, 0, bbWidth-radiusCircle_-1, 0, colorBox);
			gdImageLine(im, bbWidth-1, radiusCircle_, bbWidth-1, bbHeight-radiusCircle_-1, colorBox);
			gdImageLine(im, bbWidth-radiusCircle_-1, bbHeight-1, radiusCircle_, bbHeight-1, colorBox);
			gdImageLine(im, 0, bbHeight-radiusCircle_-1, 0, radiusCircle_, colorBox);
			gdImageSetThickness(im, stroke_-1);
			gdImageArc(im, bbWidth-radiusCircle_, radiusCircle_, 2*radiusCircle_, 2*radiusCircle_, 270, 360, colorBox);
			gdImageArc(im, bbWidth-radiusCircle_, bbHeight-radiusCircle_, 2*radiusCircle_, 2*radiusCircle_, 0, 90, colorBox);
			gdImageArc(im, radiusCircle_, bbHeight-radiusCircle_, 2*radiusCircle_, 2*radiusCircle_, 90, 180, colorBox);
			gdImageArc(im, radiusCircle_, radiusCircle_, 2*radiusCircle_, 2*radiusCircle_, 180, 270, colorBox);
			gdImageSetThickness(im, 1);
			int offsetHeight = stroke_;
			int brect[8];
			int strIndex = 0;
			for ( size_t its = 0; its < textStamp_["static"].Size(); ++its, ++strIndex ) {
				// Строки статических надписей штампа
				offsetHeight += bbStrings[strIndex][1].As<int>();
// 				gdImageStringFT(im, &brect[0], color_, const_cast<char*>(fontName_.c_str()), fontSizeStatic_, 0.0, stroke_+offsetStatic+4, offsetHeight, const_cast<char*>(textStamp_["static"][strIndex]["text"].As<string>().c_str()));
				gdImageStringFT(im, &brect[0], color_, const_cast<char*>(fontName_.c_str()), fontSizeStatic_, 0.0, (bbWidth - bbStrings[strIndex][0].As<int>())/2, offsetHeight, const_cast<char*>(textStamp_["static"][strIndex].As<string>().c_str()));
				offsetHeight += 2;
			}
			for ( size_t its = 0; its < textStamp_["sign"].Size(); ++its, ++strIndex ) {
				// Строки собственно штампа
				offsetHeight += bbStrings[strIndex][1].As<int>();
				gdImageStringFT(im, &brect[0], color_, const_cast<char*>(fontName_.c_str()), fontSize_, 0.0, stroke_+4, offsetHeight, const_cast<char*>(textStamp_["sign"][its].As<string>().c_str()));
				offsetHeight += 2;
			}
		}
		//
		QPDFObjectHandle image = QPDFObjectHandle::newStream(&pdf_); // Штамп добавляемый в pdf документ
		{
			std::stringstream imageDict;
			imageDict << "<< /Type /XObject /Subtype /Image /ColorSpace /DeviceRGB /BitsPerComponent 8 /Width " << im->sx << " /Height " << im->sy << ">>";
			image.replaceDict(QPDFObjectHandle::parse(imageDict.str()));
 			cout << "im :   pointer = " << im << "   sy = " << im->sy << ",  " << gdImageSY(im) << "   sx = " << im->sx << ",  " << gdImageSX(im) << endl;
			string streamPixel(im->sy * im->sx * 3, 0);
			for ( int ih = 0; ih < im->sy; ++ih ) {
				for ( int iw = 0; iw < im->sx; ++iw ) {
					unsigned int colorPoint = ((im->tpixels)[ih])[iw];
					int pos = ( ih * im->sx + iw ) * 3;
					streamPixel[pos] = gdTrueColorGetRed(colorPoint);
					streamPixel[pos+1] = gdTrueColorGetGreen(colorPoint);
					streamPixel[pos+2] = gdTrueColorGetBlue(colorPoint);
				}
			}
			image.replaceStreamData(streamPixel, QPDFObjectHandle::newNull(), QPDFObjectHandle::newNull());
		}
// 		{
// 			FILE * pngout;
// 			if ( ( pngout = fopen("test.stamp.png", "wb") ) == NULL ) throw "Can't create file";
// 			gdImagePng(im, pngout);
// 			fclose(pngout);
// 		}
		gdImageDestroy(im); // Удаление графической канвы прорисовки штампа
		
		string imageName("/As15");
		std::stringstream dscContent;
		dscContent << "q " << bbWidth << " 0 0 " << bbHeight << " " << (width_-bbWidth)/2-2 << " " << 2 << " cm " << imageName << " Do Q ";
		
		for ( size_t ip = 0; ip < pages.size(); ++ip ) {
 			cout << "  page = " << ip << ",   dscContent = " << dscContent.str() << endl;
			QPDFObjectHandle resources = pages[ip].getKey("/Resources");
			if ( resources.isNull() ) {
				QPDFObjectHandle xobject = QPDFObjectHandle::newDictionary();
				xobject.replaceKey(imageName, image);
				QPDFObjectHandle resourcesImg = QPDFObjectHandle::newDictionary();
				resourcesImg.replaceKey("/XObject", xobject);
				pages[ip].replaceKey("/Resources", resourcesImg);
			}
			else {
				QPDFObjectHandle xobj = resources.getKey("/XObject");
				if ( xobj.isNull() ) {
					QPDFObjectHandle xobject = QPDFObjectHandle::newDictionary();
					resources.replaceKey("/XObject", xobject);
					xobj = resources.getKey("/XObject");
				}
				xobj.replaceKey(imageName, image);
			}
			
			QPDFObjectHandle contents = pages[ip].getKey("/Contents");
			if ( contents.isNull() ) {
				QPDFObjectHandle content = QPDFObjectHandle::newStream(&pdf_, dscContent.str());
				pages[ip].replaceKey("/Contents", content);
			}
			else {
 				cout << "  page = " << ip << ",  contents ! isNull" << endl;
				pages[ip].addPageContents(QPDFObjectHandle::newStream(&pdf_, dscContent.str()), false);
			}
		}
	}
}

}
