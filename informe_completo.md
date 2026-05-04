# INFORME DEL PROYECTO: Tank Transformer

Facultad de Ingeniería de la Universidad Nacional de Itapúa  
Ingeniería en Informática – 10mo Semestre  
Proyecto #3 de Gráficos por Computadoras

---

## 1. Funcionalidades Implementadas

El proyecto implementa un robot que se transforma entre cuatro formas: humanoide, auto, avión y barco. Las funcionalidades incluyen:

- **Aspecto 3D con primitivas:** El modelo se construye con Box, Cylinder y Sphere unidos jerárquicamente
- **Movimiento de extremidades:** Rotación de torreta/ala (A/D), elevación de cañón/ala (Q/E), apertura de escotilla (H), selección y rotación manual (TAB + U/O/Y/N/Z/B)
- **Cambio de colores:** Ciclo de colores del cuerpo (C) y orugas/pistas (click derecho)
- **Saludo con luz y sonido:** Rotación del torso, movimiento de brazo/ala, parpadeo de luces y reproducción de audio (G)
- **Resaltado de partes:** Selección de extremidades con TAB, resaltado con amarillo y nombre en HUD
- **Caminar/andar:** Movimiento con piernas balanceándose en humanoide, ruedas rotando en auto, hélice en avión (W/S)
- **Disparo:** En auto/barco proyectiles desde el cañón; en avión dos bombas desde las alas (F, solo en modo auto/avión/barco)
- **Transformación animada:** Ciclo entre humanoide → auto → avión → barco → humanoide (T)
- **Control de cámara:** Movimiento orbital con flechas y mouse, rotación con I/K/J/L, zoom con scroll
- **HUD de texto:** Modo actual, indicadores de estado, parte seleccionada, cooldown de disparo, controles en pantalla

---

## 2. Estructura del Código Base

```
TankTransformer/
├── Transformer.sln                 # Solución Visual Studio
├── Plano-de-Corte/
│   ├── main.cpp                    # Punto de entrada, shaders GLSL, callbacks GLUT
│   ├── sounds/
│   │   ├── greet.wav               # Sonido de saludo
│   │   ├── shoot.wav               # Sonido de disparo
│   │   └── walk.wav                # Sonido de caminar
│   ├── include/
│   │   ├── Math3D.h                # Biblioteca matemática (Vec3, Mat4, proyección)
│   │   ├── Component.h             # Nodo base del grafo de escena
│   │   ├── Primitives.h            # Figuras: Box, Cylinder, Sphere
│   │   ├── Tank.h                  # Clase principal con poses y animaciones
│   │   ├── Environment.h           # Cámara orbital y suelo
│   │   ├── HUD.h                   # Texto 2D en pantalla
│   │   └── ShaderManager.h         # Compilación de shaders GLSL
│   ├── GL/                         # Headers de OpenGL/GLEW/FreeGLUT
│   └── lib/                        # Bibliotecas compiladas
├── x64/Debug/                      # Ejecutable y DLLs
└── informe_completo.md             # Este documento
```

**Clases principales:**
- `Component`: Nodo jerárquico con transformación y renderizado recursivo
- `Box`, `Cylinder`, `Sphere`: Primitivas con geometría VAO/VBO
- `Tank`: Modelo completo con poses extremas (humanoide/tanque) y animaciones
- `Environment`: Cámara orbital y suelo de tablero
- `HUD`: Renderizado de texto 2D
- `ShaderManager`: Compilación y validación de shaders GLSL

**Patrón de diseño:**
- Grafo de escena plano: todas las partes son hijos directos de un nodo raíz
- Interpolación de poses: `lerp2()` interpola entre estados, `smoothstep()` suaviza
- Shaders inline: GLSL embebido en main.cpp para portabilidad

---

## 3. Dificultades Encontradas

### Transformación Animada Entre Cuatro Formas (Mayor dificultad)

La transformación fue la parte más costosa del proyecto. Las razones principales fueron:

1. **Definición de poses extremas:** Cada una de las partes principales (body, torso, head, arms, legs) necesita posición, escala, rotación y color para CUATRO estados distintos (HUMANOID, CAR, PLANE, BOAT). Encontrar valores que se vieran naturales en cada forma requirió múltiples iteraciones y prueba visual.

2. **Sincronización de visibilidad de componentes:** Diferentes componentes deben ser visibles en diferentes modos:
   - Ruedas: solo en CAR
   - Hélice: solo en PLANE
   - Timón: solo en BOAT
   - El timing de aparición/desaparición durante la interpolación es crítico

3. **Comportamientos específicos por modo:** 
   - HUMANOID: piernas balanceándose
   - CAR: ruedas rotando, cañón elevable
   - PLANE: hélice girando, dos proyectiles
   - BOAT: movimiento ondulante

4. **Jerarquía de transformaciones:** Se implementó con posiciones absolutas (jerarquía plana) para evitar acumulación de transformaciones.

**Solución:** Sistema de poses independientes para cada modo, interpolación suavizada con `smoothstep()`, máquina de estados diferenciada por modo.


### Integración de Audio con Windows Multimedia

- Usar `PlaySound()` requirió manejar rutas relativas y sincronización con animaciones
- Los archivos WAV debían estar en el directorio correcto
- Se creó una función wrapper para manejar errores

---

---

## 5. Declaración de Autoría y Ayudas Recibidas

**Participantes del proyecto:** Rodrigo Maidana, Jorge Figueredo, José Ferreira, Pedro Kazlauskas, María Esquivel.

**Afirmamos que:**

El código fuente, la lógica de programación y todas las decisiones de diseño fueron desarrollados por nuestro equipo. 

**Ayuda recibida de Inteligencia Artificial (Claude AI):**
- Consulta en estructura de clases y patrones de diseño
- Asistencia en depuración de shaders GLSL
- Validación de algoritmos de interpolación y easing
- Revisión de la documentación
- Consulta general sobre buenas prácticas de OpenGL

---

---

*Informe del Proyecto #3 de Gráficos por Computadoras*
