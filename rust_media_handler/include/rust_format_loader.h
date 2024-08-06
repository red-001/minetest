#pragma once
#include <cstdint>
static_assert(sizeof(uint32_t) == 4);

namespace rust_format
{
namespace loader
{

struct state;

enum class status_code : uint32_t
{
	Ok,
	IOError,
	DecodingError,
	EncodingError,
	ParameterError,
	LimitError,
	NotImplementedError,
	InvalidColorFormat,

	Count
};

inline static constexpr char* status_code_name[static_cast<uint32_t>(status_code::Count)] = {
	"Ok",
	"IOError",
	"DecodingError",
	"EncodingError",
	"ParameterError",
	"LimitError",
	"NotImplementedError",
	"InvalidColorFormat"
};

enum class image_format : uint32_t
{
	RGB8,
	RGBA8,
};

struct image_meta
{
	image_format format;
	uint32_t width;
	uint32_t height;
};

}
}

extern "C" rust_format::loader::state *rust_format_loader_image_loader_init();
extern "C" void rust_format_loader_image_loader_destory(rust_format::loader::state *state);

extern "C" rust_format::loader::status_code rust_format_loader_image_loader_from_data(rust_format::loader::state *state, const char *file_path, const void *data, size_t len);
extern "C" const char* rust_format_loader_image_loader_err(rust_format::loader::state *state);
extern "C" bool rust_format_loader_image_loader_get_meta(rust_format::loader::state *state, rust_format::loader::image_meta *meta_out);
extern "C" bool rust_format_loader_image_loader_get_data(rust_format::loader::state *state, void *data_out, size_t len);
