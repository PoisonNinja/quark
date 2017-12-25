target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/intel-8259.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/irqchip.cc
)