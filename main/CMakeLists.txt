set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDEDIRS "")
set(COMPONENT_REQUIRES ulp bootloader_support)

register_component()					   
					   
# ULP support additions to component CMakeLists.txt.

# 1. The ULP app name must be unique (if multiple components use ULP).
set(ulp_app_name ulp_${COMPONENT_NAME})

# 2. The ULP sources (in separate directory!)
set(ulp_riscv_sources "../ULP/main_ulp.c")

# 3. All the main project source files which will automatically include the generated ${ulp_app_name}.h file
set(ulp_exp_dep_srcs "main.c")

# 4. Call function to build ULP binary and embed in project using the argument values above.
ulp_embed_binary(${ulp_app_name} "${ulp_riscv_sources}" "${ulp_exp_dep_srcs}")
