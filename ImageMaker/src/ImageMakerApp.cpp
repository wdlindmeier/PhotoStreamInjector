#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Xml.h"
#include "cinder/gl/TextureFont.h"
#include <time.h> 

using namespace ci;
using namespace ci::app;
using namespace std;

struct ImageData
{
    string lat = "";
    string lng = "";
    float raceOther = 0.f;
    float raceWhite = 0.f;
    float raceBlack = 0.f;
    float raceAsian = 0.f;
    float raceLatino = 0.f;
    float genderMale = 0.f;
    float genderFemale = 0.f;
    float incomeLevel = 0.f;
    float ageMale = 0.f;
    float ageFemale = 0.f;
};

class ImageMakerApp : public AppNative {
  public:
    void prepareSettings(Settings *settings);
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    int mCurrentImage;
    vector<ImageData> mImageDatas;

    Font				mFontLarge;
    Font				mFontSmall;
    gl::TextureFontRef	mTextureFontLarge;
    gl::TextureFontRef	mTextureFontSmall;
    
    gl::Texture         mDateTexture;
    gl::Texture         mLatTexture;
    gl::Texture         mLngTexture;
    
};

void ImageMakerApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(400, 256);
}

void ImageMakerApp::setup()
{
    mCurrentImage = 0;
    
    mFontLarge = Font( "Heiti TC", 24 );
    mFontSmall = Font( "Heiti TC", 14 );

    mTextureFontLarge = gl::TextureFont::create( mFontLarge );
    mTextureFontSmall = gl::TextureFont::create( mFontSmall );

    XmlTree doc( loadFile( "/Users/bill/Documents/ITP/AIT-Creative Misuse/image_data.xml" ) );
    XmlTree images = doc.getChild( "opt" );

    for( XmlTree::Iter child = images.begin(); child != images.end(); ++child )
    {
        ImageData imgData;
        
        XmlTree dataNode = child->getChild("data");
        
        imgData.lat = dataNode.getAttributeValue<string>("lat");
        imgData.lng = dataNode.getAttributeValue<string>("lng");
        imgData.raceOther = dataNode.getAttributeValue<float>("race_other");
        imgData.raceWhite = dataNode.getAttributeValue<float>("race_white");
        imgData.raceAsian = dataNode.getAttributeValue<float>("race_asian");
        imgData.raceBlack = dataNode.getAttributeValue<float>("race_black");
        imgData.raceLatino = dataNode.getAttributeValue<float>("race_latino");
        
        imgData.genderMale = dataNode.getAttributeValue<float>("gender_male");
        imgData.genderFemale = dataNode.getAttributeValue<float>("gender_female");
        
        imgData.incomeLevel = dataNode.getAttributeValue<float>("income_level");
        
        imgData.ageMale = dataNode.getAttributeValue<float>("age_male");
        imgData.ageFemale = dataNode.getAttributeValue<float>("age_female");
        mImageDatas.push_back(imgData);
    }
    
    
    time_t rawtime;
    struct tm * timeinfo;
    
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    
    char dateBuffer[80];
    strftime(dateBuffer, 80, "%m\n%d\n%y", timeinfo);
    string dateString(dateBuffer);
    
    TextBox tboxDate = TextBox().alignment( TextBox::CENTER ).font( mFontLarge ).text( dateString );
    Vec2f textSize = tboxDate.measure();
    tboxDate.size( Vec2f( 72.0f, textSize.y ) );
	tboxDate.setColor( Colorf( 0.5f, 0.5f, 0.5f ) );
	tboxDate.setBackgroundColor( ColorA( 0,0,0,0 ) );

    mDateTexture = gl::Texture( tboxDate.render() );
}

void ImageMakerApp::mouseDown( MouseEvent event )
{
}

void ImageMakerApp::update()
{
    
    if (mCurrentImage < mImageDatas.size())
    {
        ImageData imgData = mImageDatas[mCurrentImage];

        // Create the lat and long textures
        for (int i = 0; i < 2; ++i)
        {
            string locString;
            if (i == 0)
            {
                locString = imgData.lat;
            }
            else
            {
                locString = imgData.lng;
            }
            if(locString.find("-") == -1)
            {
                locString = string(" ") + locString;
            }
            if (locString.size() > 7)
            {
                locString = locString.substr(0, 7);
            }
            
            string vertLocString = "";
            for (int i = 0; i < locString.size(); ++i)
            {
                vertLocString += locString.substr(i,1);
                vertLocString += "\n";
            }
            
            TextBox tboxLoc = TextBox().alignment( TextBox::CENTER ).font( mFontSmall ).text( vertLocString );
            Vec2f textSize = tboxLoc.measure();
            tboxLoc.size( Vec2i( 24, textSize.y ) );
            tboxLoc.setColor( Colorf( 0.5f, 0.5f, 0.5f ) );
            tboxLoc.setBackgroundColor( ColorA( 0,0,0,0 ) );
            if (i == 0)
            {
                mLatTexture = gl::Texture( tboxLoc.render() );
            }
            else
            {
                mLngTexture = gl::Texture( tboxLoc.render() );
            }
        }
    }
}

void ImageMakerApp::draw()
{
    if (mCurrentImage >= mImageDatas.size())
    {
        exit(0);
        return;
    }
    
	// clear out the window with white
	gl::clear( Color( 1, 1, 1 ) );
    gl::enableAlphaBlending();
    
    // Draw the date
    gl::color(Color::white());
    float vertMargin = (256.0 - mDateTexture.getHeight()) * 0.5f;
    gl::draw(mDateTexture,
             Area(0,0,mDateTexture.getWidth(),mDateTexture.getHeight()),
             Rectf(0,vertMargin,mDateTexture.getWidth(),vertMargin+mDateTexture.getHeight()));

    
    if (mCurrentImage < mImageDatas.size())
    {
        ImageData data = mImageDatas[mCurrentImage];
    }
    
    // Draw demographic data
    ImageData imgData = mImageDatas[mCurrentImage];
    float barHeight = 256.0/4.0;
    float barWidth = 256.0f;

    // Column 1: gender
    float barX = 72.0;
    float xOff = (barWidth * (imgData.genderFemale * 0.01));
    // Female (pink)
    gl::color(Colorf(1.f,0.65f,0.65f));
    gl::drawSolidRect(Rectf(barX, 0, barX + xOff, barHeight));
    // Male (blue)
    barX += xOff;
    xOff = (barWidth * (imgData.genderMale * 0.01));
    gl::color(Colorf(0.3f,0.55f,0.85f));
    gl::drawSolidRect(Rectf(barX, 0, barX + xOff, barHeight));


    // Column 2: race (asian (red), white, black, latino (green), other)
    // Asian (red)
    barX = 72.0;
    xOff = (barWidth * (imgData.raceAsian * 0.01));
    gl::color(Colorf(0.9,0,0));
    gl::drawSolidRect(Rectf(barX, barHeight, barX + xOff, barHeight*2));
    // White
    barX += xOff;
    xOff = (barWidth * (imgData.raceWhite * 0.01));
    gl::color(Colorf(1,1,1));
    gl::drawSolidRect(Rectf(barX, barHeight, barX + xOff, barHeight*2));
    // Black
    barX += xOff;
    xOff = (barWidth * (imgData.raceBlack * 0.01));
    gl::color(Colorf(0,0,0));
    gl::drawSolidRect(Rectf(barX, barHeight, barX + xOff, barHeight*2));
    // Latino (green)
    barX += xOff;
    xOff = (barWidth * (imgData.raceLatino * 0.01));
    gl::color(Colorf(1.0,0.6,0));
    gl::drawSolidRect(Rectf(barX, barHeight, barX + xOff, barHeight*2));
    // Other (gray)
    barX += xOff;
    xOff = (barWidth * (imgData.raceOther * 0.01));
    gl::color(Colorf(0.5f,0.5f,0.5f));
    gl::drawSolidRect(Rectf(barX, barHeight, barX + xOff, barHeight*2));
    
    // Column 3: age black, yellow
    barX = 72.0;
    float scalarMaleAge = imgData.ageMale / 76.0f;
    float scalarFemaleAge = imgData.ageFemale / 81.0f;
    float ageValue = (scalarMaleAge + scalarFemaleAge) / 2.0f;
    xOff = (barWidth * ageValue);
    gl::color(Colorf(0.0f,0.0f,0.0f));
    gl::drawSolidRect(Rectf(barX, barHeight*2, barX + xOff, barHeight*3));
    barX += xOff;
    xOff = barWidth - xOff;
    gl::color(Colorf(1.0,0.95,0));
    gl::drawSolidRect(Rectf(barX, barHeight*2, barX + xOff, barHeight*3));

    // Column 4: income black, green
    barX = 72.0;
    xOff = (barWidth * (imgData.incomeLevel * 0.01));
    gl::color(Colorf(0.1,0.8,0));
    gl::drawSolidRect(Rectf(barX, barHeight*3, barX + xOff, barHeight*4));
    barX += xOff;
    xOff = barWidth - xOff;
    gl::color(Colorf(0.4f,0.3f,0.0f));
    gl::drawSolidRect(Rectf(barX, barHeight*3, barX + xOff, barHeight*4));
    
    // Draw the loc
    gl::color(Color::white());
    vertMargin = ((256.0 - mLatTexture.getHeight()) * 0.5f) - 5;
    gl::draw(mLatTexture,
             Area(0,0,mLatTexture.getWidth(),mLatTexture.getHeight()),
             Rectf(340,vertMargin,364,vertMargin + mLatTexture.getHeight()));

    vertMargin = ((256.0 - mLngTexture.getHeight()) * 0.5f) - 5;;
    gl::draw(mLngTexture,
             Area(0,0,mLngTexture.getWidth(),mLngTexture.getHeight()),
             Rectf(364,vertMargin,388,vertMargin + mLngTexture.getHeight()));

    // Save out
    time_t t;
    long timestamp = time(&t);
    string filename = string("demo_") + to_string(timestamp) + "_" + to_string(mCurrentImage);
    string filePath("/Users/bill/Documents/ITP/AIT-Creative Misuse/output/" + filename);
    writeImage(filePath + ".png", copyWindowSurface());
    mCurrentImage++;

}

CINDER_APP_NATIVE( ImageMakerApp, RendererGl )
