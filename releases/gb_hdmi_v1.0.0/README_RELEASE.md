# gb_hdmi v1.0.0 release manifest (no binary blobs in git)

Este repositorio **no versiona binarios** en esta carpeta para evitar errores de actualización de rama ("Binary files are not supported").

## Artefactos esperados
Estos archivos se generan localmente al compilar:
- `gb_hdmi.uf2`
- `gb_hdmi.hex`
- `gb_hdmi.bin`
- `gb_hdmi.elf`

## Cómo generarlos
```bash
cd src/gb_vga
mkdir -p build && cd build
PICO_SDK_PATH=../pico-sdk cmake -G Ninja ..
cmake --build . -j
```

## Cómo publicar release fuera del repo
Sube los artefactos generados como assets en GitHub Releases (no en el árbol git).
