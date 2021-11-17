#include <stdlib.h>

#include "types.h"
#include "rxlntest.h"
#include "platform.h"
#include "rxas.h"

typedef struct module {
    BOOL loaded;
    char *name;
    bin_space segment;
} module;

module loadModule(char *fileName, module *program);
void   getProcedures(module *program);


void test(char *fileName) {

    module program;

    printf("DEBUG: Binary file %s will be loaded. \n", fileName);

    program = loadModule(fileName, &program);

    if (program.loaded) {
        getProcedures(&program);
    }  else {
        fprintf(stderr, "ERROR: Binary file %s could not be loaded.", fileName);
        exit(-1);
    }
}

void getProcedures(module *program) {

    size_t ii = 0;

    chameleon_constant  *c_entry;
    proc_constant       *p_entry;

    while (ii < (*program).segment.const_size) {
        c_entry = (chameleon_constant *) ((*program).segment.const_pool + ii);

        if (c_entry->type == EXPOSE_PROC_CONST) {
            p_entry = (proc_constant *) ((*program).segment.const_pool + ((expose_proc_constant *) c_entry) ->procedure);

            if (((expose_proc_constant *) c_entry)->imported) {
                fprintf(stdout, "Found external exposed procedure '%s' \n", p_entry->name);
            } else {
                fprintf(stdout, "Found internal exposed procedure '%s' \n", p_entry->name);
            }
        }

        ii += c_entry->size_in_pool;
    }
}

module loadModule(char *fileName, module *program) {
    FILE *fp;

    fp = openfile(fileName, "rxbin", NULL, "rb");

    if (fp) {
        // read numeric fields
        fread(&(*program).segment.globals,    1, sizeof(int),    fp);
        fread(&(*program).segment.inst_size,  1, sizeof(size_t), fp);
        fread(&(*program).segment.const_size, 1, sizeof(size_t), fp);

        // skip code area
        fseek(fp, (long)(sizeof(bin_code) * (*program).segment.inst_size), SEEK_CUR);

        // read constants
        (*program).segment.const_pool = calloc((*program).segment.const_size, 1);
        fread((*program).segment.const_pool, 1, (*program).segment.const_size, fp);

        program->name = fileName;
        program->loaded = TRUE;

        fclose(fp);
    }

    return (*program);
}

