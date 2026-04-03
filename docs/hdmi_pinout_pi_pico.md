# Pinout propuesto Pi Pico para salida HDMI directa (PicoDVI/picodvi_pmod0_cfg)

Este pinout corresponde a la configuración `picodvi_pmod0_cfg` (PicoDVI) usada en `gb_hdmi`.

## TMDS (DVI sobre conector HDMI)

- `GPIO6` → TMDS Clock+
- `GPIO7` → TMDS Clock-
- `GPIO2` → TMDS Data2+
- `GPIO3` → TMDS Data2-
- `GPIO4` → TMDS Data1+
- `GPIO5` → TMDS Data1-
- `GPIO0` → TMDS Data0+
- `GPIO1` → TMDS Data0-

> Nota: las señales son pares diferenciales emulados desde GPIO single-ended y requieren la red resistiva/etapa física de salida tipo PicoDVI.

## Captura de video Game Boy (sin cambios respecto al firmware base)

- `GPIO18` VSYNC input
- `GPIO17` HSYNC input
- `GPIO16` Pixel clock input
- `GPIO15` Data_1 input
- `GPIO14` Data_0 input

## Compatibilidad de pines

Con `picodvi_pmod0_cfg`, los GPIO de TMDS (0-7 pares no continuos) **no colisionan** con la captura Game Boy usada por este firmware (`GPIO14-18`).

Aun así, para hardware final, debe verificarse en el PCB el ruteo diferencial y la etapa resistiva TMDS.
