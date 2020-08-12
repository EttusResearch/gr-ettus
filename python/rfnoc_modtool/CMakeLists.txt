###RFNoC Modtool

include(GrPython)

GR_PYTHON_INSTALL(FILES
	    __init__.py
        cmakefile_editor.py
        code_generator.py
        grc_xml_generator.py
        modtool_add.py
        modtool_base.py
        modtool_disable.py
        modtool_help.py
        modtool_info.py
        modtool_makexml.py
        modtool_newmod.py
        modtool_rm.py
        modtool_rename.py
        parser_cc_block.py
        scm.py
        templates.py
        util_functions.py
        DESTINATION ${GR_PYTHON_DIR}/ettus/rfnoc_modtool
        COMPONENT "utils"
)

set(GR_PKG_RFNOC_MODTOOL_DATA_DIR ${GR_PKG_DATA_DIR}/rfnoc_modtool)
install(DIRECTORY rfnoc-newmod
    DESTINATION ${GR_PKG_RFNOC_MODTOOL_DATA_DIR}
    COMPONENT "utils"
    USE_SOURCE_PERMISSIONS
    )

#################################################################
# Create and install the rfnoc_modtool conf file
#################################################################
file(TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}/${GR_PKG_RFNOC_MODTOOL_DATA_DIR}/python/rfnoc-newmod" newmoddir)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/rfnocmodtool.conf.in
	${CMAKE_CURRENT_BINARY_DIR}/rfnocmodtool.conf
@ONLY)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/rfnocmodtool.conf
    DESTINATION ${GR_PREFSDIR}
    COMPONENT "utils"
)
