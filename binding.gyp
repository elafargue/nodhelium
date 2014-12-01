{
  "targets": [
    {
      "target_name": "helium",
      "sources": [ "helium.cc", "helium_wrapper.cc" ],
      'conditions': [
        [ 'OS=="mac"', {
          'libraries': [ '/usr/local/lib/libhelium.dylib'],
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
        ]

    }
  ]
}
