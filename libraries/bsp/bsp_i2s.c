#include "bsp_i2s.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "bsp_i2s.pio.h"

// 全局变量
static int i2sSoundSm;
static uint i2sSoundOffset;
static int dmaSoundTx;

void bsp_i2s_init(void)
{
    // 初始化PIO管脚
    pio_gpio_init(BSP_I2S_SOUND_PIO, BSP_I2S_DIN_PIN);
    pio_gpio_init(BSP_I2S_SOUND_PIO, BSP_I2S_BCK_PIN);
    pio_gpio_init(BSP_I2S_SOUND_PIO, BSP_I2S_LRCK_PIN);

    // 获取状态机和偏移量
    i2sSoundSm = pio_claim_unused_sm(BSP_I2S_SOUND_PIO, true);
    i2sSoundOffset = pio_add_program(BSP_I2S_SOUND_PIO, &i2s_pio_program);

    // 获取PIO的缺省设置
    pio_sm_config cfg = i2s_pio_program_get_default_config(i2sSoundOffset);

    // 数据管脚
    sm_config_set_out_pins(&cfg, BSP_I2S_DIN_PIN, 1);

    // 附加设置管脚
    sm_config_set_sideset_pins(&cfg, BSP_I2S_BCK_PIN);

    // 设置从左拉取数据（MSB），自动拉取，拉取阈值32位
    sm_config_set_out_shift(&cfg, false, true, 32);

    // 合并输入输出的FIFO
    sm_config_set_fifo_join(&cfg, PIO_FIFO_JOIN_TX);

    pio_sm_init(BSP_I2S_SOUND_PIO, i2sSoundSm, i2sSoundOffset, &cfg);

    // 设置I2S管脚的方向为输出
    uint pinMask = (1u << BSP_I2S_DIN_PIN) | (3u << BSP_I2S_BCK_PIN);

    pio_sm_set_pindirs_with_mask(BSP_I2S_SOUND_PIO, i2sSoundSm, pinMask, pinMask);

    // 将I2S各管脚清0
    pio_sm_set_pins(BSP_I2S_SOUND_PIO, i2sSoundSm, 0);

    // 设置状态机频率
    // 分频器 = 主机频率 * 256 / ( 2 * 声道数 * 声音位数 * 声音频率)
    // 分频器 = 主机频率 * 256 / (2 * 2 * 16 * 44100)
    // 分频器 = 主机频率 * 4 / 44100
    uint32_t freqDiv = clock_get_hz(clk_sys) * 4 / BSP_I2S_FREQ;
    pio_sm_set_clkdiv_int_frac(BSP_I2S_SOUND_PIO, i2sSoundSm, freqDiv >> 8, freqDiv & 0xFF);

    printf("I2S sound freq: %d, divider: %d\n", BSP_I2S_FREQ, freqDiv);
    pio_sm_set_enabled(BSP_I2S_SOUND_PIO, i2sSoundSm, true);

    // DMA
    dmaSoundTx = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(dmaSoundTx);
    channel_config_set_dreq(&c, pio_get_dreq(BSP_I2S_SOUND_PIO, i2sSoundSm, true));
    dma_channel_configure(dmaSoundTx, &c, (uint32_t *)&(BSP_I2S_SOUND_PIO->txf[i2sSoundSm]), NULL, 0, false);
}

void bsp_i2s_output(const uint32_t *sound, size_t len, bool wait)
{
    if (wait)
    {
        dma_channel_wait_for_finish_blocking(dmaSoundTx);
    }
    else if (dma_channel_is_busy(dmaSoundTx))
        return;
    dma_channel_transfer_from_buffer_now(dmaSoundTx, (uint32_t *)sound, len);
}

void i2sSoundOutputDmaBlocking(const uint32_t *sound, size_t len)
{
    dma_channel_wait_for_finish_blocking(dmaSoundTx);
    dma_channel_transfer_from_buffer_now(dmaSoundTx, (uint32_t *)sound, len);
}
