#include "mpack.h"

void
validate_data(const char *data, size_t size)
{
    mpack_tree_t tree;
    mpack_tree_init_data(&tree, data, size);
    mpack_tree_parse(&tree);
    mpack_node_t root = mpack_tree_root(&tree);

    if (mpack_node_type(root) != mpack_type_array || mpack_node_array_length(root) != 2) {
        fprintf(stderr, "Invalid data: root should be an array of length 2\n");
        mpack_tree_destroy(&tree);
        return;
    }

    for (size_t i = 0; i < 2; ++i) {
        mpack_node_t map = mpack_node_array_at(root, i);
        if (mpack_node_type(map) != mpack_type_map || mpack_node_map_count(map) != 1) {
            fprintf(stderr, "Invalid data: each item should be a map of length 1\n");
            mpack_tree_destroy(&tree);
            return;
        }
    }

    mpack_tree_destroy(&tree);
}

void
write_data_to_buf(char **data, size_t *size)
{

    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, data, size);

    mpack_start_array(&writer, 2); // Start an array of 2 key-value maps

    mpack_start_map(&writer, 1); // Start a map of 1 key-value pair
    mpack_write_cstr(&writer, "key1");
    mpack_write_int(&writer, 123);
    mpack_finish_map(&writer);

    mpack_start_map(&writer, 1); // Start a map of 1 key-value pair
    mpack_write_cstr(&writer, "key2");
    mpack_start_array(&writer, 2); // Start an array of 2 values
    mpack_write_double(&writer, 456.78);
    mpack_write_cstr(&writer, "value");
    mpack_finish_array(&writer);
    mpack_finish_map(&writer);

    mpack_finish_array(&writer);

    if (mpack_writer_destroy(&writer) != mpack_ok) {
        fprintf(stderr, "An error occurred encoding the data!\n");
        return 1;
    }
}

int
main()
{
    char * data;
    size_t size;

    write_data_to_buf(&data, &size);

    // validate the data...
    validate_data(data, size);

    // clean up
    free(data);

    return 0;
}
