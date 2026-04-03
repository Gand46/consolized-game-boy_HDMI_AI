# Evaluación técnica: migrar de VGA a HDMI directo (RP2040 / Pi Pico)

Fecha: 2026-04-03

## 1) Estado actual del proyecto

El firmware actual **no genera HDMI/DVI**, genera video VGA con `pico_scanvideo_dpi` (RGB paralelo + HSync/VSync):

- En `src/gb_vga/CMakeLists.txt` se compila y linkea con `pico_scanvideo_dpi`.
- En `src/gb_vga/gb_vga.c` está definido el modo 640x480@60 con temporización VGA (`vga_timing_640x480_gb`, `vga_mode_640x480_3x_scale`).

Adicionalmente, el hardware/BOM de este proyecto ya contempla un **conversor externo VGA→HDMI** (`GANA VGA to HDMI Audio Video Converter`), por lo que el puerto HDMI actual del montaje depende de ese conversor y no de TMDS directo desde el RP2040.

## 2) Conclusión ejecutiva (sin ambigüedad)

Sí, **es técnicamente posible** eliminar VGA y sacar señal “HDMI” directa desde Pi Pico, pero en RP2040 esto es realmente salida **DVI sobre conector HDMI** (TMDS de video, típicamente sin audio/CEC).

Para este proyecto concreto, el cambio es **viable pero no trivial**: requiere rediseño de firmware de video y de hardware de salida.

## 3) Validación de paquetes/librerías para HDMI en Pi Pico

Se validaron referencias y disponibilidad remota (repos accesibles por `git ls-remote`) para el stack más relevante:

### A. `raspberrypi/pico-extras` (actual usado para VGA)
- Estado: válido y mantenido para utilidades extra de SDK.
- Hallazgo clave: `pico_scanvideo_dpi` se describe como soporte estable para salida **RGB paralelo/VGA/DPI**, no TMDS HDMI/DVI directo.
- Uso recomendado en este proyecto: **mantener para VGA**, no es la base de HDMI directo.

### B. `Wren6991/PicoDVI` (base C/C++ para DVI sobre RP2040)
- Estado: repo accesible y activo; contiene `software/libdvi` y configuraciones de pines (`common_dvi_pin_configs.h`).
- Hallazgo clave: es la base más directa para DVI/HDMI en RP2040 con PIO+DMA+TMDS por software.
- Uso recomendado: **candidato principal** para migración en este proyecto (porque el firmware actual está en C con pico-sdk).

### C. `adafruit/PicoDVI` (fork Arduino)
- Estado: accesible y versionado (library.properties versión 1.3.2).
- Hallazgo clave: el propio repo se define como fork orientado a Arduino/Adafruit_GFX y aclara requisitos de overclock/limitaciones.
- Uso recomendado: útil como referencia, pero **menos ideal** para integrar en este repo CMake/pico-sdk puro.

## 4) Impacto técnico real para implementar HDMI directo

## 4.1 Firmware (obligatorio)
1. Reemplazar pipeline `scanvideo_dpi` por pipeline DVI/TMDS (`libdvi`/PicoDVI).
2. Ajustar reloj del sistema a frecuencias típicas del modo DVI (p.ej. 252 MHz para 640x480p60 pixel-doubled), con validación de estabilidad por lote de chips.
3. Rediseñar generador de scanlines/framebuffer para formato de entrada de encoder TMDS.
4. Presupuestar recursos:
   - PIO (3 SM en una instancia)
   - DMA (múltiples canales dedicados)
   - Carga de CPU relevante (normalmente un core muy ocupado)
   - RAM para buffers

## 4.2 Hardware (obligatorio)
1. Quitar dependencia de DAC/resistencias VGA + conversor VGA→HDMI.
2. Añadir etapa física TMDS (red resistiva/circuito recomendado por diseños PicoDVI probados) y ruteo diferencial corto/cuidado.
3. Reasignar GPIO para lanes TMDS + clock según configuración seleccionada.
4. Validar integridad de señal con diferentes pantallas (compatibilidad no 100% garantizada en todas).

## 4.3 Funcionalidad esperable
- Video digital directo por HDMI (a nivel protocolo de video DVI).
- Posible pérdida de compatibilidad universal respecto a usar conversor externo dedicado.
- Audio HDMI no forma parte del camino estándar de PicoDVI de forma transparente (considerarlo fuera de alcance inicial).

## 5) Riesgos y mitigaciones

1. **Riesgo de estabilidad por overclock** (silicio RP2040 y temperatura).
   - Mitigación: calificación por lotes, stress test prolongado, perfil térmico.
2. **Riesgo de compatibilidad entre monitores/TV**.
   - Mitigación: matriz de validación real con varios displays y cables.
3. **Riesgo de consumo de recursos (PIO/DMA/CPU)** afectando otras funciones.
   - Mitigación: presupuesto temprano y prototipo mínimo antes de portar OSD/efectos.
4. **Riesgo de retrabajo PCB**.
   - Mitigación: prototipo intermedio en placa DVI conocida antes de cerrar nuevo PCB.

## 6) Plan mínimo recomendado (orden sugerido)

Fase 0 — PoC externo (sin tocar PCB final)
- Levantar demo mínima 640x480p60 DVI en hardware PicoDVI/pico_sock.
- Confirmar estabilidad de reloj y compatibilidad con al menos 3 displays.

Fase 1 — Integración firmware
- Integrar `libdvi` en rama nueva.
- Portar path de render actual (game window + OSD) a scanline callback de DVI.
- Medir FPS, latencia y carga en ambos cores.

Fase 2 — Integración hardware
- Definir pinout TMDS final y esquemático.
- Prototipo PCB de salida TMDS.
- Validación eléctrica/funcional (incluyendo hot-plug y cableado real).

Fase 3 — Cierre
- Retirar conversor VGA→HDMI del BOM.
- Publicar firmware/uf2 y guía de montaje actualizada.

## 7) Veredicto final

- **Sí es posible** ir a HDMI directo en este proyecto.
- **No es un cambio solo de software**: hay cambios obligatorios de hardware + firmware.
- Camino recomendado: base `PicoDVI/libdvi` para RP2040, no `pico_scanvideo_dpi`.
- El enfoque más seguro es migración por fases con PoC previo antes de rediseñar PCB/BOM.
