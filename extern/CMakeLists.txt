include(FetchContent)

FetchContent_Declare(
  sanisizer
  GIT_REPOSITORY https://github.com/LTLA/sanisizer
  GIT_TAG master # ^0.1.0
)

FetchContent_MakeAvailable(sanisizer)
