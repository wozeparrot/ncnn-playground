project('ncnn-playground', 'cpp',
  version : '0.1.0',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

# dependencies
ncnn = dependency('ncnn')
opencv = dependency('opencv4')
cpp_mjpeg_streamer = dependency('nadjieb_mjpeg_streamer')

executable('ncnn-playground', [
  'main.cpp',
  'utils.cpp',
  'nanodet/nanodet.cpp'
  ],
  dependencies : [ncnn, opencv, cpp_mjpeg_streamer],
  install : true
)
