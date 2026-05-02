# INFORME DEL PROYECTO: Tank Transformer

## Datos del Proyecto

| Campo              | Detalle                                           |
|--------------------|---------------------------------------------------|
| **Nombre**         | Tank Transformer                                  |
| **Lenguaje**       | C++ 17                                            |
| **Graficos**       | OpenGL 3.3 Core Profile (GLSL 330)                |
| **Bibliotecas**    | FreeGLUT, GLEW, Windows Multimedia (winmm)        |
| **IDE**            | Visual Studio 2022 (v143)                         |
| **Plataforma**     | Windows 10/11, x64                                |
| **Repositorio**    | GitHub                                            |

---

## 1. Descripcion General

El proyecto consiste en un **tanque militar 3D que se transforma en un humanoide** (estilo Transformer), desarrollado completamente con OpenGL moderno (shaders GLSL) y C++. No se utiliza ninguna biblioteca matematica externa: toda la algebra lineal (vectores, matrices, proyeccion, lookAt) esta implementada desde cero en `Math3D.h`.

El modelo se construye uniendo **tres tipos de figuras primitivas** (Box, Cylinder, Sphere) mediante un grafo de escena jerarquico, donde cada parte tiene posicion, rotacion, escala y color propios.

---

## 2. Arquitectura del Codigo

### Estructura de archivos

```
TankTransformer/
├── Transformer.sln                    # Solucion Visual Studio
├── Plano-de-Corte/
│   ├── main.cpp                       # Punto de entrada, shaders, callbacks GLUT
│   ├── sounds/                        # Archivos de audio WAV personalizados
│   │   ├── greet.wav                  # Sonido de saludo
│   │   ├── shoot.wav                  # Sonido de disparo
│   │   └── walk.wav                   # Sonido de caminar
│   ├── include/
│   │   ├── Math3D.h                   # Biblioteca matematica propia (Vec3, Mat4)
│   │   ├── Component.h                # Nodo base del grafo de escena
│   │   ├── Primitives.h               # Figuras: Box, Cylinder, Sphere
│   │   ├── Tank.h                     # Clase principal del transformer
│   │   ├── Environment.h              # Camara orbital y suelo
│   │   ├── HUD.h                      # Texto 2D en pantalla
│   │   └── ShaderManager.h            # Compilador de shaders GLSL
│   ├── GL/                            # Headers de OpenGL (freeglut, glew)
│   └── lib/                           # Bibliotecas compiladas (.lib)
├── x64/Debug/                         # Ejecutable y DLLs
└── INFORME.md                         # Este archivo
```

### Clases principales

| Clase            | Archivo           | Responsabilidad                                    |
|------------------|-------------------|----------------------------------------------------|
| `Component`      | Component.h       | Nodo base jerarquico con transformacion y draw recursivo |
| `Box`            | Primitives.h      | Cubo unitario (36 vertices, VAO estatico)          |
| `Cylinder`       | Primitives.h      | Cilindro parametrico con tapas                     |
| `Sphere`         | Primitives.h      | Esfera UV parametrica                              |
| `Tank`           | Tank.h            | Modelo completo: poses, animaciones, acciones      |
| `Environment`    | Environment.h     | Camara orbital, suelo de tablero                   |
| `HUD`            | HUD.h             | Overlay de texto 2D con glutBitmapCharacter        |
| `ShaderManager`  | ShaderManager.h   | Compilacion y linkeo de shaders GLSL               |

### Patron de diseno

- **Grafo de escena**: Cada parte del modelo es un `Component` hijo del nodo raiz `root`. Se usa jerarquia plana (todos hijos directos de root) para que las posiciones sean absolutas.
- **Interpolacion de poses**: Cada parte tiene dos estados extremos (humanoide y tanque). La funcion `lerp2()` interpola entre ambos, y `smoothstep()` suaviza la curva.
- **Shaders inline**: Los shaders GLSL estan embebidos como cadenas en `main.cpp` para maxima portabilidad.

---

## 3. Requisitos Implementados

### Req. 1 — Aspecto uniendo figuras primitivas (4p)

El modelo usa **tres tipos de primitivas**:
- **Box**: cuerpo, torso/torreta, cabeza, brazos, piernas/orugas, baldosas del suelo
- **Cylinder**: canon, 8 ruedas (4 por lado), antena
- **Sphere**: faro delantero, proyectiles (balas)

En total se crean **14+ componentes** que forman tanto el humanoide como el tanque.

### Req. 2 — Mover extremidades/ruedas/puertas por separado (4p)

| Tecla | Accion                                    |
|-------|-------------------------------------------|
| A / D | Rotar torreta izquierda/derecha           |
| Q / E | Elevar/descender el canon                 |
| H     | Abrir/cerrar escotilla (cabeza)           |

Las ruedas rotan automaticamente durante la caminata en modo tanque. Las piernas se balancean en modo humanoide.

### Req. 3 — Cambiar colores con mouse o teclado (4p)

| Control              | Accion                                  |
|----------------------|-----------------------------------------|
| Tecla **C**          | Ciclar color del cuerpo (5 colores)     |
| **Click derecho**    | Ciclar color de las orugas (3 colores)  |

Paleta de colores del cuerpo: verde militar, rojo oscuro, azul, amarillo, gris.

### Req. 4 — Saludo con habla y luz (4p)

Al presionar **G**:
1. El torso rota de lado a lado (animacion de saludo, 3 segundos)
2. El brazo izquierdo se agita arriba y abajo
3. El faro delantero **parpadea** en amarillo (simula hablar)
4. Se reproduce un **sonido personalizado** (`sounds/greet.wav` via PlaySound)
5. La luz se apaga al terminar

### Req. 5 — Resaltar extremidad seleccionada (1p)

Al presionar **X** se cicla entre las partes del modelo. La parte seleccionada se dibuja en **color amarillo** (`Vec3{1,1,0}`) en lugar de su color normal. Tambien se resalta automaticamente la parte que se esta moviendo (torreta, canon, escotilla).

### Req. 6 — Caminar/andar con tecla y parar con otra (2p)

| Tecla | Accion                               |
|-------|--------------------------------------|
| **W** | Iniciar caminata                     |
| **S** | Detener caminata                     |

- **Modo humanoide**: las piernas se balancean con `sin(walkPhase)`.
- **Modo tanque**: las 8 ruedas rotan continuamente.
- En ambos modos, el modelo avanza en el eje Z.
- Se reproduce un **sonido de pasos** (`sounds/walk.wav`) al iniciar, y se detiene al parar.

### Req. 7 — Disparar con sonido y efecto (2p)

Al presionar **F** (solo en modo tanque):
1. Se crea un **proyectil esfera naranja** que sale del canon
2. El proyectil avanza a velocidad constante y desaparece tras 2 segundos
3. Se reproduce un **sonido de disparo** (`sounds/shoot.wav` via PlaySound)
4. Tiene **cooldown de 0.5 segundos** entre disparos (se muestra "Recargando..." en el HUD)

### Req. 8 — Transformacion con animacion (2p)

Al presionar **T**:
- Si esta en modo humanoide, se transforma en tanque
- Si esta en modo tanque, se transforma en humanoide
- La transformacion es una **interpolacion suavizada** (smoothstep) de 0 a 1
- Todas las partes cambian posicion, escala, rotacion y color simultaneamente
- Las ruedas, antena y faro aparecen/desaparecen a mitad de la transicion

### Req. 10 — Mover el entorno (1p)

| Control              | Accion                                 |
|----------------------|----------------------------------------|
| **Flechas arriba/abajo** | Mover camara adelante/atras         |
| **Flechas izq/der**      | Mover camara lateralmente            |
| **I / K**                 | Inclinar camara arriba/abajo         |
| **J / L**                 | Rotar camara izquierda/derecha       |
| **Mouse drag**            | Rotar camara orbital                 |
| **Scroll**                | Acercar/alejar camara                |

El movimiento de flechas desplaza el punto objetivo de la camara en el plano XZ, relativo a la orientacion actual de la camara.

### Req. 11 — Texto en pantalla (1p)

El HUD muestra en tiempo real:
- Titulo del proyecto ("TANK TRANSFORMER")
- Modo actual (TANQUE / HUMANOIDE)
- Indicadores de estado: [CAMINANDO], [HABLANDO], [LUZ ON]
- Parte seleccionada actualmente
- Indicador de recarga del disparo
- Panel de controles completo (esquina inferior izquierda)

Se implementa con `glutBitmapCharacter` y proyeccion ortografica 2D temporal.

### Req. 13 — Codigo con clases, comentarios (1p)

- **7 clases** organizadas en archivos separados por responsabilidad
- Todos los archivos tienen **comentarios detallados** en espanol explicando:
  - Proposito de cada clase y funcion
  - Parametros y valores de retorno
  - Algoritmos usados (interpolacion, generacion de geometria, etc.)
  - Que requisito cubre cada funcionalidad
- Nombres descriptivos para variables y funciones

---

## 4. Parte que Mas Costo

La parte mas dificil del proyecto fue la **transformacion animada entre humanoide y tanque** (Requisito 8). Las razones principales:

1. **Definir las poses extremas**: Cada una de las 7 partes principales necesita posicion, escala, rotacion y color tanto en modo humanoide como en modo tanque. Encontrar valores que se vean bien en ambos estados requirio mucha prueba y error.

2. **Jerarquia plana vs jerarquica**: Originalmente se intento usar una jerarquia padre-hijo real (torso hijo de body, brazos hijos de torso, etc.), pero las transformaciones acumuladas hacian que los valores de posicion fueran confusos durante la interpolacion. La solucion fue usar **jerarquia plana** (todos hijos de root) con posiciones absolutas.

3. **Smoothstep y timing**: La interpolacion lineal pura se veia robotica. Agregar `smoothstep()` mejoro la fluidez, pero requirio ajustar los thresholds de visibilidad (cuando aparecen/desaparecen ruedas, cabeza, etc.) para que la transicion se viera natural.

4. **Sincronizar animaciones**: Cuando la transformacion ocurre mientras el modelo camina o saluda, las animaciones interfieren entre si. Fue necesario separar claramente las responsabilidades de cada animacion en `update()`.

La segunda parte mas costosa fue la **biblioteca matematica propia** (Math3D.h), ya que implementar `mat4Perspective`, `mat4LookAt` y `mat4Rotate` desde cero (sin GLM) requirio entender la matematica subyacente de OpenGL a nivel de columna-mayor y verificar que los resultados coincidieran con los de GLM.

---

## 5. Controles Completos

| Tecla / Input     | Accion                                    |
|-------------------|-------------------------------------------|
| T                 | Transformar humanoide <-> tanque          |
| W                 | Iniciar caminata                          |
| S                 | Detener caminata                          |
| A / D             | Rotar torreta izquierda / derecha         |
| Q / E             | Elevar / descender canon                  |
| H                 | Abrir / cerrar escotilla                  |
| G                 | Saludar (habla + luz)                     |
| F                 | Disparar (solo modo tanque)               |
| C                 | Cambiar color del cuerpo                  |
| X                 | Ciclar seleccion de parte                 |
| Flechas           | Mover entorno (camara)                    |
| I / J / K / L     | Rotar / inclinar camara                   |
| Mouse drag        | Rotar camara orbital                      |
| Click derecho     | Cambiar color de orugas                   |
| Scroll            | Acercar / alejar camara                   |
| ESC               | Salir del programa                        |

---

## 6. Tecnologias y Dependencias

| Tecnologia        | Version     | Uso                                       |
|-------------------|-------------|-------------------------------------------|
| C++               | C++17       | Lenguaje principal                        |
| OpenGL            | 3.3 Core    | Renderizado 3D con shaders               |
| GLSL              | 330         | Vertex y fragment shaders                 |
| FreeGLUT          | 3.x         | Ventana, input, loop principal            |
| GLEW              | 2.x         | Carga de extensiones OpenGL               |
| Windows MM        | Win32 API   | Reproduccion de sonidos (PlaySound)       |
| Visual Studio     | 2022 v143   | Compilacion y depuracion                  |

---

## 7. Como Compilar y Ejecutar

1. Abrir `Transformer.sln` en Visual Studio 2022
2. Seleccionar configuracion **Debug | x64**
3. Compilar con **Ctrl+Shift+B** (o Build > Build Solution)
4. Ejecutar con **F5** (o Debug > Start Debugging)
5. Asegurarse de que `freeglut.dll` y `glew32.dll` esten en la carpeta del ejecutable (`x64/Debug/`)

---

## 8. Firmas

**Desarrolladores del proyecto:**

Firma: ___________________________  
Nombre:  
Fecha:  

Firma: ___________________________  
Nombre:  
Fecha:  

Firma: ___________________________  
Nombre:  
Fecha:  

---

*Documento generado como parte del requisito 9 del proyecto Tank Transformer.*
