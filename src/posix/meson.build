posix_srcs = files(
    'terminal.c',
    'simmem.c',
    'init.c',
)

defines = [
    '-DPOSIX_SIM_MEM_SIZE=' + get_option('posix_mem_size').to_string()
]

header_search_paths = include_directories(
    '.',
)

port_info = {
    'defs': defines,
    'sources': posix_srcs,
    'includes': header_search_paths,
}
