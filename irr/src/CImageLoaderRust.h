// Copyright 2024 red-001 <red-001@outlook.ie>
// This file is part of the "IrrlichtMT Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#pragma once

#include "IImageLoader.h"

namespace irr
{
namespace video
{
/*!
	Surface Loader that uses rust image-rs to load the image
*/
class CImageLoaderRust : public IImageLoader
{
public:
	//! constructor
	CImageLoaderRust();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".tga")
	bool isALoadableFileExtension(const io::path &filename) const override;

	//! returns true if the file maybe is able to be loaded by this class
	bool isALoadableFileFormat(io::IReadFile *file) const override;

	//! creates a surface from the file
	IImage *loadImage(io::IReadFile *file) const override;
};

} // end namespace video
} // end namespace irr
