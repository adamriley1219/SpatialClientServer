#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Graphics/Rgba.hpp"



// NOT A TEXTURE, Don't use to draw with.
class Image
{
public:
	Image();
	explicit Image( const char* imageFilePath, bool flip = false );
	explicit Image( const IntVec2& size );
	~Image();

	bool TryLoadFromFile( const char* imageFilePath, bool flip = false );
	void LoadFromFile( const char* imageFilePath, bool flip = false );

	bool SaveToFile( const std::string& filepath );

	Rgba GetTexelColor( uint texelX, uint texelY ) const;
	Rgba GetTexelColor( const IntVec2& texelCoords ) const;
	const IntVec2&		GetDimensions() const;
	const std::string&	GetFilePath()	const;
	uint GetWidth() const;
	uint GetHeight() const;
	uint GetBytesPerPixel() const;
	unsigned char* GetBuffer() const;
	uint GetSize() const;

	void SetTexelColor( int texelX, int texelY, const Rgba& color );
	void SetTexelColor( const IntVec2& texelCoords, const Rgba& color );
	void SetFilepath( const std::string& filepath );

	void Resize( const IntVec2& dimensions );

private:
	IntVec2					m_dimensions = IntVec2::ONE;
	unsigned char*			m_rawData = nullptr;
	std::string				m_imageFilePath;
};

