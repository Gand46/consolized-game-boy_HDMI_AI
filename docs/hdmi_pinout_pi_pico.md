# Pinout final Pi Pico para transición 100% DVI/HDMI (video)

Este pinout corresponde a `picodvi_pmod0_cfg` (PicoDVI) y a la captura de video Game Boy usada por `gb_hdmi`.

## TMDS (video DVI sobre conector HDMI)

- `GPIO6`  → TMDS Clock+
- `GPIO7`  → TMDS Clock-
- `GPIO2`  → TMDS Data2+
- `GPIO3`  → TMDS Data2-
- `GPIO4`  → TMDS Data1+
- `GPIO5`  → TMDS Data1-
- `GPIO0`  → TMDS Data0+
- `GPIO1`  → TMDS Data0-

## Captura de video Game Boy

- `GPIO18` VSYNC input
- `GPIO17` HSYNC input
- `GPIO16` Pixel clock input
- `GPIO15` Data_1 input
- `GPIO14` Data_0 input

## GPIO sugeridos para futura ruta de audio HDMI (proyección)

> Esta sección es de planeación; en esta entrega no se embebe audio en HDMI.

- `GPIO10` I2S BCLK (propuesto)
- `GPIO11` I2S LRCLK/WS (propuesto)
- `GPIO12` I2S DATA (propuesto)

## Notas de implementación

- El diseño requiere etapa física TMDS resistiva/cableado diferencial compatible PicoDVI.
- La transición actual es HDMI-only para video (DVI/TMDS), con VGA deshabilitado en build.
