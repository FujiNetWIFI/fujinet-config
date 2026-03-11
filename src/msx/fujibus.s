SECTION code_fujibus
ORG 0xB900

EXTERN _fuji_bus_call
SECTION fujibus_anchor
ORG 0xBFF9
    jmp _fuji_bus_call