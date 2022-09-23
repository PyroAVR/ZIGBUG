project(
    'ZIGBUG',
    'c',
    default_options: [
        'optimization=s',
        'unity=on',
        'buildtype=minsize',
        'b_lto=true'
    ],
    version: '0.0.1',
)

# version control tags
sw_ver = vcs_tag(
    command: ['git', 'rev-parse', 'HEAD'],
    input: 'include/sw_ver.h.in',
    output: 'sw_ver.h',
)

# Set compiler options
cc = meson.get_compiler('c')
if cc.get_argument_syntax() == 'gcc'
    add_project_arguments([
        '-ffunction-sections',
        '-fdata-sections',
    ], language: 'c')
    add_project_link_arguments([
        '-Wl,--gc-sections'
    ], language: 'c')
endif


# Enable / Disable features
if get_option('ihex').enabled()
    # do stuff
endif

if get_option('memdump').enabled()
    #do stuff
endif

includes = ['include']

subdir('src' / 'common')

includes += 'includes' in common ? common['includes'] : []

foreach port:get_option('ports')
    subdir('src' / port)
    defs = 'defs' in port_info ? port_info['defs'] : []
    inc = 'includes' in port_info ? port_info['includes'] : []
    src = 'sources' in port_info ? port_info['sources'] : []
    port_exe = executable(
        port,
        [
            src,
            common['sources'],
        ],
        c_args: defs,
        include_directories: [inc, includes]
    )
endforeach
