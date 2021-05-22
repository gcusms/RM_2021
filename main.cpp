#include "Basis/RM_Link/RM_Link.h"

int main()
{
    unique_ptr<RM_Link> run = make_unique<RM_Link>();
    run->run();

    return EXIT_SUCCESS;
}
