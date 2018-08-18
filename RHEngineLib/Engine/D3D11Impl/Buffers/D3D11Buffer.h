#pragma once
#include "../../Definitions.h"
#include "../../../stdafx.h"
namespace RHEngine {

    struct D3D11BufferInfo 
    {
        unsigned int size;
        D3D11_USAGE usage;
        D3D11_BIND_FLAG bindingFlags;
        unsigned int cpuAccessFlags;
    };

    /*!
        @class D3D1Buffer
        @brief Base D3D hardware buffer class.

        This class represents GPU memory buffer.
    */
    class D3D11Buffer
    {
    public:
        /*!
            @brief Initializes D3D hardware buffer.

            Allocates hardware buffer of some non-negative size(in bytes) with custom usage,
            binding, cpu access, flags, element structure size(in bytes) and initial data if provided.
        */
        D3D11Buffer(ID3D11Device* device, const D3D11BufferInfo &info,
            unsigned int miscFlags = 0, unsigned int elementSize = 0, const D3D11_SUBRESOURCE_DATA* initialData = nullptr);

        /*!
            Releases memory held by hardware buffer.
        */
        ~D3D11Buffer();

        /*!
            @brief Updates data inside buffer.

            @note If size is negative, than all buffer data will be updated, otherwise only part of it.
        */
        virtual void Update(ID3D11DeviceContext* context, void* data, int size = -1);

        /*!
            Sets debugging info for this buffer
        */
        void SetDebugName(const String& name);
    protected:
        ID3D11Buffer*	m_pBuffer = nullptr;
        unsigned int	m_uiSize;
        String          m_sDebugName;
    };
};