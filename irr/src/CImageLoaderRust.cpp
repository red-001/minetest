// Copyright 2024 red-001 <red-001@outlook.ie>
// This file is part of the "IrrlichtMT Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CImageLoaderRust.h"

#include "IReadFile.h"
#include "IMemoryReadFile.h"
#include "CImage.h"
#include "os.h"

#include "rust_format_loader.h"

namespace irr
{
namespace video
{

//! constructor
CImageLoaderRust::CImageLoaderRust()
{
#ifdef _DEBUG
	setDebugName("CImageLoaderRust");
#endif
}

//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".tga")
bool CImageLoaderRust::isALoadableFileExtension(const io::path &filename) const
{
	return core::hasFileExtension(filename, "bmp")      || 
		core::hasFileExtension(filename, "jpg", "jpeg") ||
		core::hasFileExtension(filename, "png")         ||
		core::hasFileExtension(filename, "tga");
}

//! returns true if the file maybe is able to be loaded by this class
bool CImageLoaderRust::isALoadableFileFormat(io::IReadFile *file) const
{
	// assume we can load the file
	return true;
}

//! creates a surface from the file
IImage *CImageLoaderRust::loadImage(io::IReadFile *file) const
{
	static_assert(sizeof(long) <= sizeof(size_t), "CImageLoaderRust::loadImage assumes a positive `long` can fit inside a `size_t`");

	using unique_loader_state_t = std::unique_ptr<rust_format::loader::state, decltype(&rust_format_loader_image_loader_destory)>;
	unique_loader_state_t state(rust_format_loader_image_loader_init(), &rust_format_loader_image_loader_destory);

	rust_format::loader::status_code status;
	if (file->getType() == io::ERFT_MEMORY_READ_FILE)
	{
		io::IMemoryReadFile* memory_file = dynamic_cast<io::IMemoryReadFile*>(file);

		if (!memory_file || memory_file->getSize() < 0)
			return nullptr;

		status = rust_format_loader_image_loader_from_data(state.get(), memory_file->getFileName().c_str(), memory_file->getBuffer(), memory_file->getSize());
	}
	else
	{
		long file_length = file->getSize();
		if (file_length <= 0)
			return nullptr;

		std::vector<u8> file_contents;
		file_contents.resize(file_length);

		size_t length_read = file->read(file_contents.data(), file_contents.size());

		// back out if we couldn't read the full file
		if (length_read != file_contents.size())
		{
			os::Printer::log(
				"Failed to load image from source filesystem",
				file->getFileName().c_str(),
				ELL_ERROR);
			return nullptr;
		}

		status = rust_format_loader_image_loader_from_data(state.get(), file->getFileName().c_str(), file_contents.data(), file_contents.size());
	}

	if (status != rust_format::loader::status_code::Ok)
	{
		os::Printer::log(
			"Failed to load image",
			rust_format::loader::status_code_name[static_cast<uint32_t>(status)],
			ELL_ERROR);

		const char *rust_error_msg = rust_format_loader_image_loader_err(state.get());

		if (rust_error_msg)
		{
			os::Printer::log(
				"Rust error message",
				rust_error_msg,
				ELL_ERROR);
		}

		return nullptr;
	}

	rust_format::loader::image_meta meta = {};

	if (!rust_format_loader_image_loader_get_meta(state.get(), &meta))
	{
		os::Printer::log("Failed to get image metadata", ELL_ERROR);

		return nullptr;
	}

	if (!checkImageDimensions(meta.width, meta.height))
	{
		os::Printer::log("Rejecting image file with unreasonable size.", ELL_ERROR);

		return nullptr;
	}

	// create irr surface
	core::dimension2d<u32> dim;
	dim.Width = meta.width;
	dim.Height = meta.height;

	ECOLOR_FORMAT format = meta.format == rust_format::loader::image_format::RGB8 ? ECF_R8G8B8 : ECF_A8R8G8B8;

	CImage *image = new CImage(format, dim);

	if (!image)
	{
		os::Printer::log("Failed to allocate memory for Irr image.", ELL_ERROR);

		return nullptr;
	}

	u32 image_data_size = CImage::getDataSizeFromFormat(format, dim);

	if (!rust_format_loader_image_loader_get_data(state.get(), image->getData(), image_data_size))
	{
		os::Printer::log("Failed to copy image over from Rust side.", ELL_ERROR);

		delete image;
		image = nullptr;

		return nullptr;
	}

	return image;
}

//! creates a loader which is able to load TGA, BMP, PNG and JPEG surfaces using rust.
IImageLoader *createImageLoaderRust()
{
	return new CImageLoaderRust;
}

} // end namespace video
} // end namespace irr
