use core::slice;
use image::{DynamicImage, ImageError, ImageFormat, ImageReader};
use std::ffi::{c_char, c_uchar, CStr, CString, OsStr};
use std::io::{BufRead, Cursor, Read, Seek};
use std::path::Path;

#[repr(u32)]
pub enum CStatusCode {
    Ok,
    IOError,
    DecodingError,
    EncodingError,
    ParameterError,
    LimitError,
    NotImplementedError,
    InvalidColorFormat,
}

#[repr(u32)]
pub enum CImageFormat {
    RGB8,
    RGBA8,
}

#[repr(C)]
pub struct CImageMeta {
    format: CImageFormat,
    width: u32,
    height: u32,
}

fn image_error_to_status(image_error: ImageError) -> CStatusCode {
    match image_error {
        ImageError::Decoding(_) => CStatusCode::DecodingError,
        ImageError::Encoding(_) => CStatusCode::EncodingError,
        ImageError::Parameter(_) => CStatusCode::ParameterError,
        ImageError::Limits(_) => CStatusCode::LimitError,
        ImageError::Unsupported(_) => CStatusCode::NotImplementedError,
        ImageError::IoError(_) => CStatusCode::IOError,
    }
}

#[derive(Default)]
pub struct LoaderState {
    image: Option<DynamicImage>,
    error_message: CString,
}

impl LoaderState {
    fn decode<R: Read + Seek + BufRead>(
        &mut self,
        reader: ImageReader<R>,
    ) -> Result<DynamicImage, ImageError> {
        let map_to_image_err = |e| -> ImageError { ImageError::IoError(e) };
        reader
            .with_guessed_format()
            .map_err(map_to_image_err)?
            .decode()
    }

    fn decode_to_status<R: Read + Seek + BufRead>(
        &mut self,
        reader: ImageReader<R>,
    ) -> CStatusCode {
        let decode_result = self.decode(reader);

        match decode_result {
            Ok(image) => {
                self.image = Some(Self::convert_to_irr_format(image));

                CStatusCode::Ok
            }
            Err(decode_err) => {
                let err_string = format!("Image decode error: {}", decode_err);
                self.error_message = CString::new(err_string).unwrap();

                image_error_to_status(decode_err)
            }
        }
    }

    fn convert_to_irr_format(image: DynamicImage) -> DynamicImage {
        if image.color().has_alpha() {
            let mut rgba8 = image.into_rgba8();

            for pixel in rgba8.pixels_mut() {
                pixel.0.swap(0, 2);
            }

            DynamicImage::ImageRgba8(rgba8)
        } else {
            let rgb8 = image.into_rgb8();

            DynamicImage::ImageRgb8(rgb8)
        }
    }

    pub fn get_format(&self) -> Option<CImageMeta> {
        match &self.image {
            None => None,
            Some(image) => {
                let format = match image {
                    DynamicImage::ImageRgb8(_) => CImageFormat::RGB8,
                    DynamicImage::ImageRgba8(_) => CImageFormat::RGBA8,
                    _ => todo!(),
                };
                let meta = CImageMeta {
                    format,
                    width: image.width(),
                    height: image.height(),
                };
                Some(meta)
            }
        }
    }

    pub fn get_data(&self, buffer_out: &mut [u8]) -> bool {
        match &self.image {
            None => false,
            Some(image) => {
                let image_buffer = image.as_bytes();
                if image_buffer.len() == buffer_out.len() {
                    buffer_out.copy_from_slice(image_buffer);
                    true
                } else {
                    false
                }
            }
        }
    }

    pub fn open_binary(&mut self, buffer: &[u8], image_path: Option<&str>) -> CStatusCode {
        self.image = None;

        let mut reader = ImageReader::new(Cursor::new(buffer));

        let extension = image_path
            .map(|p: &str| -> &Path { Path::new(p) })
            .and_then(|p| -> Option<&OsStr> { p.extension() })
            .and_then(OsStr::to_str);

        if let Some(ext) = extension {
            let low_ext = ext.to_lowercase();
            match low_ext.as_str() {
                "tga" => reader.set_format(ImageFormat::Tga),
                "bmp" => reader.set_format(ImageFormat::Bmp),
                "png" => reader.set_format(ImageFormat::Png),
                "jpg" | "jpeg" => reader.set_format(ImageFormat::Jpeg),
                _ => (),
            };
        }
        self.decode_to_status(reader)
    }
}

#[no_mangle]
pub unsafe extern "C" fn rust_format_loader_image_loader_init() -> *mut LoaderState {
    let state = LoaderState::default();
    let boxed = Box::new(state);
    Box::into_raw(boxed)
}

#[no_mangle]
pub unsafe extern "C" fn rust_format_loader_image_loader_destory(state: *mut LoaderState) {
    if !state.is_null() {
        unsafe {
            drop(Box::from_raw(state));
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn rust_format_loader_image_loader_from_data(
    c_state: *mut LoaderState,
    c_image_path: *const c_char,
    data: *const c_uchar,
    len: usize,
) -> CStatusCode {
    assert!(!c_state.is_null());
    assert!(!data.is_null());

    let mut image_path: Option<&str> = Option::None;

    if !c_image_path.is_null() {
        let cstr_image_path = unsafe { CStr::from_ptr(c_image_path) };
        let path = cstr_image_path.to_str().unwrap();
        if !path.is_empty() {
            image_path = Option::Some(path)
        }
    }

    let buffer = unsafe { slice::from_raw_parts(data, len) };

    let state = unsafe { c_state.as_mut() }.unwrap();
    state.open_binary(buffer, image_path)
}

#[no_mangle]
pub unsafe extern "C" fn rust_format_loader_image_loader_err(
    c_state: *mut LoaderState,
) -> *const c_char {
    assert!(!c_state.is_null());

    let state = unsafe { c_state.as_mut() }.unwrap();
    state.error_message.as_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn rust_format_loader_image_loader_get_meta(
    c_state: *mut LoaderState,
    meta_out: *mut CImageMeta,
) -> bool {
    assert!(!c_state.is_null());
    assert!(!meta_out.is_null());

    let state = unsafe { c_state.as_mut() }.unwrap();
    let meta_opt = state.get_format();
    if meta_opt.is_some() {
        unsafe { *meta_out = meta_opt.unwrap() };

        true
    } else {
        false
    }
}

#[no_mangle]
pub unsafe extern "C" fn rust_format_loader_image_loader_get_data(
    c_state: *mut LoaderState,
    data: *mut c_uchar,
    len: usize,
) -> bool {
    assert!(!data.is_null());
    assert!(!c_state.is_null());

    let state = unsafe { c_state.as_mut() }.unwrap();
    let buffer_out = unsafe { slice::from_raw_parts_mut(data, len) };

    state.get_data(buffer_out)
}
