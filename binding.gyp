{
  "targets": [
    {
      "target_name": "helium",
      "sources": [ "helium.cc", "helium_wrapper.cc" ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      'libraries': [ '-lhelium'],
      'conditions': [
        [ 'OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }
	]]
    }
  ]
}
