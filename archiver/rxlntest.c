#include <stdlib.h>

#include "rxlntest.h"
#include "platform.h"
#include "rxastree.h"
#include "rxas.h"

typedef struct module {
    bin_space segment;
    char *name;
    //void **globals;
} module;

void test(char *fileName) {

    module program;

    FILE *fp;

    size_t ii = 0;

    chameleon_constant *c_entry;
    proc_constant *p_entry, p_entry_linked;

    struct avl_tree_node *exposed_proc_tree = 0;

    printf("FOO> Loading file: %s \n", fileName);

    fp = openfile(fileName, "rxbin", NULL, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR opening file %s\n", fileName);
        exit(-1);
    }

    fread(&program.segment.globals,    1, sizeof(int),    fp);
    fread(&program.segment.inst_size,  1, sizeof(size_t), fp);
    fread(&program.segment.const_size, 1, sizeof(size_t), fp);

    program.segment.const_pool = calloc(program.segment.const_size, 1);

    fseek(fp, sizeof(bin_code) * program.segment.inst_size, SEEK_CUR);
    fread(program.segment.const_pool, 1, program.segment.const_size, fp);

    program.segment.module_index = 0;

    fclose(fp);

    while (ii < program.segment.const_size) {
        c_entry =
                (chameleon_constant *) (
                        program.segment.const_pool + ii);
        switch (c_entry->type) {
            case EXPOSE_PROC_CONST:
                p_entry =
                        (proc_constant *) (
                                program.segment.const_pool
                                + ((expose_proc_constant *) c_entry)
                                        ->procedure);

                if (((expose_proc_constant *) c_entry)->imported) {

                    fprintf(stdout, "Found imported procedure '%s' \n", p_entry->name);

                    if (!src_node(exposed_proc_tree,
                                  ((expose_proc_constant *) c_entry)->index,
                                  (size_t *) &p_entry_linked)) {

                        fprintf(stdout, "Unimplemented symbol: %s\n",
                                ((expose_proc_constant *) c_entry)->index);

                    }
                }

                break;

            default:;
        }

        ii += c_entry->size_in_pool;
    }
}

