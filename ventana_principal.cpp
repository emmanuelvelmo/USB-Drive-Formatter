#include <tchar.h> // Para manejar caracteres TCHAR (compatibilidad Unicode/ANSI)
#include <stdio.h> // Para funciones de entrada/salida est�ndar (printf)
#include <iomanip> //
#include <vector> //
#include <cmath> //
#include <malloc.h> //
#include "ventana_principal.h" //
#include "variables.h" //

namespace DriveFormatter
{
    // FUNCIONES
    // Funci�n para obtener 384 bytes o menos desde la unidad (seg�n la posici�n del scrollbar vertical)
    void ventana_principal::bytes_unidad_actual()
    {
        // Calcular cu�ntos bytes podemos leer (m�ximo 384 o los que queden)
        DWORD bytes_a_leer = 384; // Nuestro tama�o deseado de lectura
        uint64_t bytes_restantes = tamano_unidad_actual - desplazamiento_actual;

        if (bytes_restantes < bytes_a_leer) {
            bytes_a_leer = static_cast<DWORD>(bytes_restantes);
        }

        // Posicionarnos en el byte inicial (posicion_unidad)
        LARGE_INTEGER pos;

        pos.QuadPart = desplazamiento_actual;

        SetFilePointer(manejador_unidad, pos.LowPart, &pos.HighPart, FILE_BEGIN);

        // Realizar la lectura
        ReadFile(
            manejador_unidad,
            buffer_lectura,
            bytes_a_leer,
            &bytes_leidos, // Variable global que almacena cu�ntos bytes se leyeron realmente
            NULL
        );
    }

    // Funci�n para mostrar 384 bytes o menos en caja de texto
    void ventana_principal::actualizar_caja_texto(uint64_t posicion_referencia)
    {
        // Establecer la posici�n actual de lectura
        desplazamiento_actual = posicion_referencia;

        // Obtener 384 bytes o menos desde la unidad (rellenar con ceros para asegurar 2 cifras)
        bytes_unidad_actual();

        // Preparar el texto hexadecimal para mostrar
        std::wstring texto_hex;
        wchar_t byte_str[3]; // Buffer para cada byte (2 d�gitos + null)

        // Convertir cada byte a su representaci�n hexadecimal
        for (DWORD i = 0; i < bytes_leidos; ++i)
        {
            // Formatear cada byte como 2 d�gitos hexadecimales
            swprintf(byte_str, 3, L"%02X", buffer_lectura[i]);

            // Agregar al texto resultante
            texto_hex += byte_str;

            // Agregar espacio cada 16 bytes para formato organizado
            if ((i + 1) % 16 == 0)
            {
                texto_hex += L"\r\n"; // Nueva l�nea cada 16 bytes
            }
            else
            {
                texto_hex += L" "; // Espacio entre bytes
            }
        }

        // Mostrar el texto en el control EDIT
        //SetWindowText(editor_hex, texto_hex.c_str());
    }

    // Funci�n para
    void ventana_principal::actualizar_barra_progreso()
    {
        // Calcular progreso actual


        // Mostrar progreso en barra

    }

    // Funci�n para formatear unidad completa
    void ventana_principal::formatear_unidad()
    {
        // Acceder a unidad actual seleccionada


        // Reemplazar todos los bytes en unidad por 0x00
        //for ()
        //{
            //


            // Actualizar la barar de progreso
            //actualizar_barra_progreso();
        //}

        // Reiniciar barra de progreso a cero


        // Colocar handler al inicio
        posicion_handler = 0;

        // Actualizar vista hexadecimal
        actualizar_caja_texto(0);
    }

    // Funci�n para calcular posici�n en unidad a partir del handler de scrollbar vertical
    void ventana_principal::posicion_unidad_vscrollbar()
    {
        // Capturar posici�n de handler
        //posicion_handler = GetScrollPos(barra_desplazamiento, SB_CTL);

        // Calcular posici�n en unidad
        uint64_t posicion_unidad = static_cast<uint64_t>(round(static_cast<double>(tamano_unidad_actual) * (static_cast<double>(posicion_handler) / altura_vscrollbar)));

        // Actualizar vista hexadecimal con la unidad actual seleccionado
        actualizar_caja_texto(posicion_unidad);
    }

    // Funci�n para registrar datos de la unidad seleccionada (ej. tama�o en bytes)
    void ventana_principal::datos_unidad_actual()
    {
        // Cerrar manejador si estaba abierto previamente
        if (manejador_unidad != INVALID_HANDLE_VALUE)
        {
            CloseHandle(manejador_unidad);
            manejador_unidad = INVALID_HANDLE_VALUE;
        }

        // Abrir la unidad f�sica en modo lectura
        manejador_unidad = CreateFileW(
            unidad_actual, // Ruta al dispositivo (ej. "\\\\.\\C:")
            GENERIC_READ, // Solo lectura
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // Compartir acceso
            nullptr, // Sin atributos de seguridad
            OPEN_EXISTING, // Abrir solo si existe
            FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN, // Acceso sin cache
            nullptr // Sin plantilla
        );

        // Obtener tama�o de la unidad (alternativa simple sin winioctl.h)
        LARGE_INTEGER tam_unidad;

        GetFileSizeEx(manejador_unidad, &tam_unidad);

        tamano_unidad_actual = tam_unidad.QuadPart;

        // Inicializar buffer de lectura (sin tama�o fijo)
        if (buffer_lectura != nullptr)
        {
            _aligned_free(buffer_lectura);

            buffer_lectura = nullptr;
        }
    }

    // Actualizar combobox con la lista de unidades disponibles
    void ventana_principal::actualizar_combobox()
    {
        // Limpiar contenido previo del combobox
        combobox1->Items->Clear();

        // Recorremos solo las unidades existentes
        for (const auto& unidad : lista_unidades)
        {
            // Formatear a "X:\" usando el dato del vector
            wchar_t display[10];
            swprintf(display, 10, L"%c:\\", unidad[4]);  // Extrae la letra

            // Agregar al combobox
            combobox1->Items->Add(gcnew System::String(display));
        }

        // Auto-selecci�n si hay elementos
        if (lista_unidades.size() > 0)
        {
            combobox1->SelectedIndex = 0;
        }

        // Obtener tama�o de bytes de unidad
        datos_unidad_actual();

        // Actualizar vista hexadecimal con la unidad actual seleccionado
        actualizar_caja_texto(0);
    }

    // Funci�n para detectar y registrar unidades disponibles
    void ventana_principal::directorios_unidades()
    {
        lista_unidades.clear();  // Limpiar el vector

        DWORD drives = GetLogicalDrives(); // 

        for (char letter = 'A'; letter <= 'Z'; ++letter)
        {
            if (drives & (1 << (letter - 'A')))
            {
                // A�adir al vector din�mico
                wchar_t unidad[10];
                swprintf(unidad, 10, L"\\\\.\\%c:", letter);

                lista_unidades.push_back(unidad);
            }
        }

        // Actualizar combobox
        actualizar_combobox();
    }

    void ventana_principal::cambiar_unidad()
    {
        // Obtener letra de la unidad seleccionada (formato "C:")
        wchar_t unidad_seleccionada[4] = { 0 };
        //LRESULT indice_val = SendMessage(combo_unidades, CB_GETCURSEL, 0, 0);
        //SendMessage(combo_unidades, CB_GETLBTEXT, indice_val, (LPARAM)unidad_seleccionada);

        // Actualizar variable global (convertir a formato "\\.\C:")
        swprintf(unidad_actual, MAX_PATH, L"\\\\.\\%c:", unidad_seleccionada[0]);

        // Colocar handler al inicio
        posicion_handler = 0;
        //SetScrollPos(barra_desplazamiento, SB_CTL, 0, TRUE);

        // Obtener tama�o de bytes de unidad
        datos_unidad_actual();

        // Actualizar vista hexadecimal
        actualizar_caja_texto(0);
    }
}