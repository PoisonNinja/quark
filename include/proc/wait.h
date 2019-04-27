#pragma once

/*
 * wait status encoding
 *
 * 32-bit unsigned integer
 *
 * Bits 16-23 reserved for reason:
 *   * 0 == NORMAL
 *   * 1 == SIGNAL
 *   * 2 == STOPPED
 *   * 4 == CONT.
 *
 * Bits 8-15 reserved for exit code
 * Bits 0 - 7 reserved for signal #
 */

#define WNOHANG (1 << 0)
#define WUNTRACED (1 << 1)
#define WCONTINUED (1 << 2)

#define __WREASON_NORMAL (0 << 0)
#define __WREASON_SIGNAL (1 << 0)
#define __WREASON_STOPPED (1 << 1)
#define __WREASON_CONTINUE (1 << 2)
#define __WREASON(status) (((status) >> 16) & 0xFF)

#define __WEXITSTATUS(status) (((status) >> 8) & 0xFF)
#define __WTERMSIG(status) ((status)&0x7F)
#define __WSTOPSIG(status) (__WTERMSIG(status))

#define WIFEXITED(status) (__WREASON(status) == __WREASON_NORMAL)
#define WEXITSTATUS(status) (__WEXITSTATUS(status))
#define WIFSIGNAL(status) (__WREASON(status) == __WREASON_SIGNAL)
#define WTERMSIG(status) (__WTERMSIG(status))
#define WIFSTOPPED(status) (__WREASON(status) == __WREASON_STOPPED)
#define WSTOPSIG(status) (__WSTOPSIG(status))
#define WIFCONTINUTED(status) (__WREASON(status) == __WREASON_CONTINUE)

constexpr unsigned WGENERATE(unsigned reason, unsigned code, unsigned sig)
{
    return (((reason)&0xFF) << 16) | (((code)&0xFF) << 8) | ((sig)&0xFF);
}
