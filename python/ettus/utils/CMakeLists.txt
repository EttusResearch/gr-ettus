include(GrPython)

GR_PYTHON_INSTALL(
	PROGRAMS
	rfnocmodtool
	DESTINATION ${GR_RUNTIME_DIR}
	COMPONENT "utils"
)
