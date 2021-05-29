#ifndef BUILDTYPE_HPP
#define BUILDTYPE_HPP

#if defined (NDEBUG) && !defined (FORCEDEBUGMODE)
constexpr bool IsReleaseBuild = true;
constexpr bool IsDebugBuild   = false;
#else
constexpr bool IsReleaseBuild = false;
constexpr bool IsDebugBuild   = true;
#endif

#endif