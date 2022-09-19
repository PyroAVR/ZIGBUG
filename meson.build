project(
    'ZIGBUG',
    'c',
    default_options: [
        'optimization=s',
        'unity=on',
        'buildtype=minsize',
        'b_lto=true'
    ],
    verison = '0.0.1'
)

# version control tags
sw_ver = vcs_tag(
    command: ['git', 'rev-parse', 'HEAD'],
    input: 'include/sw_ver.h.in',
    output: 'include/sw_ver.h',
)
# Set compiler options
cc = meson.get_compiler('c')
if cc.get_argument_syntax() == 'gcc'
    add_project_arguments([
        '-ffunction-sections',
        '-fdata-sections',
    ])
    add_project_link_arguments([
        '-Wl,--gc-sections'
    ])
endif


# Enable / Disable features
if meson.get_option('ihex')
    # do stuff
endif

if meson.get_option('dumpmem')
    #do stuff
endif


foreach port in meson.get_option('ports')
    # do stuff
endforeach
