#include "Engine/Core/Graphics/Image.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Graphics/Rgba.hpp"


#pragma warning( disable: 4100 ) // unreferenced parameter
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image_write.h"

//--------------------------------------------------------------------------
/**
* Image
*/
Image::Image( const char* imageFilePath, bool flip )
{
	LoadFromFile( imageFilePath, flip );
}


//--------------------------------------------------------------------------
/**
* Image
*/
Image::Image()
{
	m_dimensions = IntVec2::ONE;
	m_rawData = new unsigned char[4];
	m_rawData[0] = (unsigned char) -1;
	m_rawData[1] = (unsigned char) -1;
	m_rawData[2] = (unsigned char) -1;
	m_rawData[3] = (unsigned char) -1;
	m_imageFilePath = "white";
}

//--------------------------------------------------------------------------
/**
* Image
*/
Image::Image(const IntVec2& size)
{
	Resize( size );
	memset( m_rawData, 255, GetSize() * GetBytesPerPixel() );
}

//--------------------------------------------------------------------------
/**
* ~Image
*/
Image::~Image()
{
	stbi_image_free( m_rawData );
}

//--------------------------------------------------------------------------
/**
* TryLoadFromFile
*/
bool Image::TryLoadFromFile(const char* imageFilePath, bool flip /*= false */)
{
	m_imageFilePath = imageFilePath;

	int imageTexelSizeX = 0; // Filled in for us to indicate image width
	int imageTexelSizeY = 0; // Filled in for us to indicate image height
	int numComponents = 0; // Filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 4; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)


	if (flip)
	{
		stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	}
	m_rawData = stbi_load(imageFilePath, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested);

	if( m_rawData == nullptr || !(numComponents >= 3 && numComponents <= 4 && imageTexelSizeX > 0 && imageTexelSizeY > 0) )
	{
		return false;
	}

	m_dimensions = IntVec2(imageTexelSizeX, imageTexelSizeY);

	return true;
}

//--------------------------------------------------------------------------
/**
* LoadFromFile
*/
void Image::LoadFromFile(const char* imageFilePath, bool flip /*= false */)
{
	m_imageFilePath = imageFilePath;

	int imageTexelSizeX = 0; // Filled in for us to indicate image width
	int imageTexelSizeY = 0; // Filled in for us to indicate image height
	int numComponents = 0; // Filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 4; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)


	if( flip )
	{
		stbi_set_flip_vertically_on_load( 1 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	}
	m_rawData = stbi_load( imageFilePath, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );

	GUARANTEE_OR_DIE( m_rawData != nullptr, Stringf("Failed to load image correctly with path: \"%s\"", imageFilePath) );
	GUARANTEE_OR_DIE( numComponents >= 3 && numComponents <= 4 && imageTexelSizeX > 0 && imageTexelSizeY > 0
		, Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath, numComponents, imageTexelSizeX, imageTexelSizeY ) );	

	// Loop through and construct image
	m_dimensions = IntVec2( imageTexelSizeX, imageTexelSizeY );
}

//--------------------------------------------------------------------------
/**
* SaveToFile
*/
bool Image::SaveToFile( const std::string& filepath )
{
	return stbi_write_png( filepath.c_str(), m_dimensions.x, m_dimensions.y, GetBytesPerPixel(), m_rawData, m_dimensions.x * GetBytesPerPixel() );
}

//--------------------------------------------------------------------------
/**
* GetTexelColor
*/
Rgba Image::GetTexelColor( const IntVec2& texelCoords ) const
{
	return GetTexelColor( texelCoords.x, texelCoords.y );
}

//--------------------------------------------------------------------------
/**
* GetTexelColor
*/
Rgba Image::GetTexelColor( uint texelX, uint texelY ) const
{
	Rgba toRet;

	uint index = texelX + texelY * m_dimensions.x;

	toRet.r = ( (float) m_rawData[index * 4] ) / 255.0f;
	toRet.g = ( (float)m_rawData[index * 4 + 1] ) / 255.0f;
	toRet.b = ( (float)m_rawData[index * 4 + 2] ) / 255.0f;
	toRet.a = ( (float)m_rawData[index * 4 + 3] ) / 255.0f;

	return toRet;
}

//--------------------------------------------------------------------------
/**
* SetTexelColor
*/
void Image::SetTexelColor( int texelX, int texelY, const Rgba& color )
{
	unsigned int index = texelX + texelY * m_dimensions.x;
	m_rawData[ index * 4 ] = ( unsigned char ) ( color.r * 255.0f );
	m_rawData[ index * 4 + 1 ] = ( unsigned char ) ( color.g * 255.0f );
	m_rawData[ index * 4 + 2 ] = ( unsigned char ) ( color.b * 255.0f );
	m_rawData[ index * 4 + 3 ] = ( unsigned char ) ( color.a * 255.0f );
}

//--------------------------------------------------------------------------
/**
* SetTexelColor
*/
void Image::SetTexelColor( const IntVec2& texelCoords, const Rgba& color )
{
	SetTexelColor( texelCoords.x, texelCoords.y, color );
}

//--------------------------------------------------------------------------
/**
* SetFilepath
*/
void Image::SetFilepath( const std::string& filepath )
{
	m_imageFilePath = filepath;
}


//--------------------------------------------------------------------------
/**
* Resize
*/
void Image::Resize( const IntVec2& dimensions )
{
	stbi_image_free( m_rawData );
	m_rawData = (unsigned char*) malloc( dimensions.x * dimensions.y * GetBytesPerPixel() );
	m_dimensions = dimensions;
}

//--------------------------------------------------------------------------
/**
* GetDimensions
*/
const IntVec2& Image::GetDimensions() const
{
	return m_dimensions;
}

//--------------------------------------------------------------------------
/**
* GetFilePath
*/
const std::string& Image::GetFilePath() const
{
	return m_imageFilePath;
}

//--------------------------------------------------------------------------
/**
* GetWidth
*/
uint Image::GetWidth() const
{
	return m_dimensions.x;
}

//--------------------------------------------------------------------------
/**
* GetHeight
*/
uint Image::GetHeight() const
{
	return m_dimensions.y;
}

//--------------------------------------------------------------------------
/**
* GetBytesPerPixel
*/
uint Image::GetBytesPerPixel() const
{
	return 4;
}

//--------------------------------------------------------------------------
/**
* GetBuffer
*/
unsigned char* Image::GetBuffer() const
{
	return m_rawData;
}

//--------------------------------------------------------------------------
/**
* GetSize
*/
unsigned int Image::GetSize() const
{
	return (unsigned int) m_dimensions.x * m_dimensions.y;
}
