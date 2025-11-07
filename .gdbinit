set pagination off
set print pretty on
set print array on
set print array-indexes on

handle SIGSEGV stop print

break vk_draw_frame if result != 0
break vk_recreate_swapchain
break vk_cleanup_swapchain

define pvk
    printf "VkResult: 0x%x (%d)\n", $arg0, $arg0
end

define pwin
    set $w = &vk_windows[$arg0]
    printf "Window %d:\n", $arg0
    printf "  initialized: %d\n", $w->initialized
    printf "  swapchain: %p\n", $w->swapchain
    printf "  image_count: %u\n", $w->swapchain_image_count
    printf "  extent: %ux%u\n", $w->swapchain_extent.width, $w->swapchain_extent.height
end