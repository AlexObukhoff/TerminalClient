//=============================================================================
// File: ComWrapper.h - COM object wrapper template class (ala ATL)
//
//    Copyright (c) 2017 Jeff Reeder
//    All Rights Reserved
//
// This straightforward template class is used for all COM object pointers used
// in this software to properly manage the lifetime of all COM objects in such
// a way that even if an exception happens, they will be auto-released when
// going out of scope, much like a smart pointer in C++11.  Unfortunately, 
// smart pointers could not be used because they rely on new/delete, and not on
// the IUnknown::Release() mechanism.  
//
// Using this wrapper object implements the RAII design pattern (Resource 
// Acquisition Is Initialization)  to cleanly manage COM object lifecycle 
// management.
//=============================================================================

#pragma once


namespace PhishMe
{
   // COM wrapper function (ATL-like) to handle auto-release
   // of COM object via the RAII design pattern.
   template <class TYPE>
   class ComWrapper
   {
   public:

      // Default blank wrapper object - primarily used in move semantics
      ComWrapper()
         : _pObject(nullptr)
      {}

      // Wrapper with an existing COM object pointer
      ComWrapper( TYPE* pComType )
         : _pObject(pComType)
      {
         if ( pComType == nullptr )   throw invalid_argument( FN_MSG( "Invalid COM object pointer" ) );
      }

      // Move constructor
      ComWrapper( ComWrapper&& wrapper )
      {
         _Move(wrapper);
      }

      // Destructor makes sure that the attached COM object is released
      virtual ~ComWrapper()
      {
         Release();
      }

      // Move assignment operator - leaves the input wrapper empty afterward
      ComWrapper& operator=( ComWrapper&& wrapper )
      {
         _Move(wrapper);
         return *this;
      }

      // Is the contained object valid?
      bool IsValid() const
      {
         return _pObject != nullptr;
      }

      // Pointer operator - returns COM object pointer as an easy shorthand
      TYPE* operator->()   { return _pObject; }

      // Get the COM object pointer
      TYPE* GetObj()   { return _pObject;   }           

      // Expliciitly release the object
      void Release()   
      {
         if ( _pObject != nullptr )
         {
            _pObject->Release();
            _pObject = nullptr;
         }
      }


   protected:

      TYPE* _pObject;


   private:

      // Move the contents of one ComWRapper<> to another (for move semantics)
      void _Move( ComWrapper& wrapper )
      {
         if ( &wrapper != this )
         {
            std::swap( _pObject, wrapper._pObject );
         }
      }
   };
}
