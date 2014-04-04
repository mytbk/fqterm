# -*- cmake -*-
#
# Utility macros for getting info from the version control system.
# Originally taken from http://emeraldviewer.googlecode.com (GPLv2)

set (PROJECT_SOURCE_DIR .)

FIND_PACKAGE(Subversion)

if (WIN32 AND NOT Subversion_FOUND)
  # The official subversion client is not available, so fall back to
  # tortoise if it is installed.
  find_program(TORTOISE_WCREV_EXECUTABLE 
    NAMES SubWCRev.exe
    PATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\TortoiseSVN;Directory]/bin"
    )

    if (NOT TORTOISE_WCREV_EXECUTABLE)
      message(INFO "TortoiseSVN was not found.")
    endif (NOT TORTOISE_WCREV_EXECUTABLE)
endif (WIN32 AND NOT Subversion_FOUND)

# Use this macro on all platforms to set _output_variable to the current SVN
# revision.
macro(vcs_get_revision _output_variable)
  if (Subversion_FOUND)
    # The included Subversion macros performs operations that require auth,
    # which breaks when building under fakeroot. Replacing with a custom 
    # command. --Ambroff
    # Subversion_WC_INFO(${PROJECT_SOURCE_DIR} _Indra)
    # set(${_output_variable} ${_Indra_WC_LAST_CHANGED_REV})

    execute_process(
      COMMAND ${Subversion_SVN_EXECUTABLE} info ../
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      OUTPUT_VARIABLE _svn_info_output
      ERROR_VARIABLE _svn_info_error
      RESULT_VARIABLE _svn_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
      )

    if (NOT ${_svn_info_result} EQUAL 0)
      message(STATUS "svn info failed: ${_svn_info_error}")
      set(${_output_variable} 0)
    else (NOT ${_svn_info_result} EQUAL 0)
      string(REGEX REPLACE 
        "(.*)?Last Changed Rev: ([0-9]+).*$"
        "\\2"
        ${_output_variable}
        ${_svn_info_output})
    endif (NOT ${_svn_info_result} EQUAL 0)
  else (Subversion_FOUND)
    if (WIN32 AND TORTOISE_WCREV_EXECUTABLE)
      # try to find TortoisSVN if we are on windows.
      execute_process(
        COMMAND ${TORTOISE_WCREV_EXECUTABLE} ../
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE _tortoise_rev_info
        ERROR_VARIABLE _tortoise_rev_info_error
        RESULT_VARIABLE _tortoise_rev_info_result
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
        )

      if (NOT ${_tortoise_rev_info_result} EQUAL 0)
        message(STATUS "Command '${_tortoise_rev_info_error} ${PROJECT_SOURCE_DIR}' failed: ${_tortoise_rev_info_error}")
        set(${_output_variable} 0)
      else (NOT ${_tortoise_rev_info_result} EQUAL 0)
        string(REGEX REPLACE 
          "(.*\n)?Last committed at revision ([0-9]+).*$"
          "\\2"
          ${_output_variable} "${_tortoise_rev_info}")
      endif (NOT ${_tortoise_rev_info_result} EQUAL 0)
    else (WIN32 AND TORTOISE_WCREV_EXECUTABLE)
      # Ruh-roh... we can't get the revision, so set it to '0'
      message(STATUS
              "No subversion client is installed. Setting build number to 0.")
      set(${_output_variable} 0)
    endif (WIN32 AND TORTOISE_WCREV_EXECUTABLE)
  endif (Subversion_FOUND)
endmacro(vcs_get_revision)
