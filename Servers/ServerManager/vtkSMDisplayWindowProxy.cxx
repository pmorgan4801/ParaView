/*=========================================================================

  Program:   ParaView
  Module:    vtkSMDisplayWindowProxy.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMDisplayWindowProxy.h"

#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h"
#include "vtkSMCommunicationModule.h"
#include "vtkSMDisplayerProxy.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMDoubleVectorProperty.h"

vtkStandardNewMacro(vtkSMDisplayWindowProxy);
vtkCxxRevisionMacro(vtkSMDisplayWindowProxy, "1.1");

//---------------------------------------------------------------------------
vtkSMDisplayWindowProxy::vtkSMDisplayWindowProxy()
{
  this->SetVTKClassName("vtkRenderWindow");

  this->RendererProxy = vtkSMProxy::New();
  this->CameraProxy = vtkSMProxy::New();
  this->CompositeProxy = vtkSMProxy::New();
  // TODO revise this: where should windowtoimage be?
  this->WindowToImage = vtkSMProxy::New();
  this->WindowToImage->ClearServerIDs();
  this->WindowToImage->AddServerID(0);
  this->ImageWriter = vtkSMProxy::New();
  this->ImageWriter->ClearServerIDs();
  this->ImageWriter->AddServerID(0);

  vtkSMDoubleVectorProperty* doubleVec;
  vtkSMIntVectorProperty* intVec;

  double zeros[3] = {0.0, 0.0, 0.0};

  // Create the SM properties for the renderer proxy

  doubleVec = vtkSMDoubleVectorProperty::New();
  doubleVec->SetCommand("SetBackground");
  doubleVec->SetNumberOfElements(3);
  doubleVec->SetElements(zeros);
  this->AddProperty("BackgroundColor", doubleVec, 0, 0);
  this->RendererProxy->AddProperty("BackgroundColor", doubleVec);
  doubleVec->Delete();

  doubleVec = vtkSMDoubleVectorProperty::New();
  doubleVec->SetCommand("ResetCamera");
  doubleVec->SetNumberOfElements(0);
  this->AddProperty("ResetCamera", doubleVec, 0, 0);
  this->RendererProxy->AddProperty("ResetCamera", doubleVec);
  doubleVec->Delete();

  // Create the SM properties for the camera proxy

  doubleVec = vtkSMDoubleVectorProperty::New();
  doubleVec->SetCommand("SetPosition");
  doubleVec->SetNumberOfElements(3);
  doubleVec->SetElement(0, 1);
  doubleVec->SetElement(1, 0);
  doubleVec->SetElement(2, 0);
  this->AddProperty("CameraPosition", doubleVec, 0, 0);
  this->CameraProxy->AddProperty("CameraPosition", doubleVec);
  doubleVec->Delete();

  doubleVec = vtkSMDoubleVectorProperty::New();
  doubleVec->SetCommand("SetFocalPoint");
  doubleVec->SetNumberOfElements(3);
  doubleVec->SetElements(zeros);
  this->AddProperty("CameraFocalPoint", doubleVec, 0, 0);
  this->CameraProxy->AddProperty("CameraFocalPoint", doubleVec);
  doubleVec->Delete();

  doubleVec = vtkSMDoubleVectorProperty::New();
  doubleVec->SetCommand("SetViewUp");
  doubleVec->SetNumberOfElements(3);
  doubleVec->SetElement(0, 0);
  doubleVec->SetElement(1, 1);
  doubleVec->SetElement(2, 0);
  this->AddProperty("CameraViewUp", doubleVec, 0, 0);
  this->CameraProxy->AddProperty("CameraViewUp", doubleVec);
  doubleVec->Delete();

  doubleVec = vtkSMDoubleVectorProperty::New();
  doubleVec->SetCommand("SetViewAngle");
  doubleVec->SetNumberOfElements(1);
  doubleVec->SetElement(0, 30);
  this->AddProperty("CameraViewAngle", doubleVec, 0, 0);
  this->CameraProxy->AddProperty("CameraViewAngle", doubleVec);
  doubleVec->Delete();

  doubleVec = vtkSMDoubleVectorProperty::New();
  doubleVec->SetCommand("SetClippingRange");
  doubleVec->SetNumberOfElements(2);
  doubleVec->SetElement(0, 0.01);
  doubleVec->SetElement(1, 1000.01);
  this->AddProperty("CameraClippingRange", doubleVec, 0, 0);
  this->CameraProxy->AddProperty("CameraClippingRange", doubleVec);
  doubleVec->Delete();

  // Create the SM properties for the renderwindow (this) proxy

  intVec = vtkSMIntVectorProperty::New();
  intVec->SetCommand("SetSize");
  intVec->SetNumberOfElements(2);
  intVec->SetElement(0, 400);
  intVec->SetElement(1, 400);
  this->AddProperty("Size", intVec);
  intVec->Delete();

  intVec = vtkSMIntVectorProperty::New();
  intVec->SetCommand("SetOffScreenRendering");
  intVec->SetNumberOfElements(1);
  intVec->SetElement(0, 1);
  this->AddProperty("OffScreenRendering", intVec);
  intVec->Delete();

}

//---------------------------------------------------------------------------
vtkSMDisplayWindowProxy::~vtkSMDisplayWindowProxy()
{
  this->RendererProxy->Delete();
  this->CameraProxy->Delete();
  this->CompositeProxy->Delete();
  this->WindowToImage->Delete();
  this->ImageWriter->Delete();
}

//---------------------------------------------------------------------------
// We overwrite this method to make sure that it is forwarded to the
// sub-proxies as well.
void vtkSMDisplayWindowProxy::UpdateVTKObjects()
{
  this->Superclass::UpdateVTKObjects();

  this->RendererProxy->UpdateVTKObjects();
  this->CameraProxy->UpdateVTKObjects();
}

//---------------------------------------------------------------------------
void vtkSMDisplayWindowProxy::CreateVTKObjects(int numObjects)
{
  if (this->ObjectsCreated)
    {
    return;
    }
  this->Superclass::CreateVTKObjects(numObjects);
  
  vtkSMCommunicationModule* cm = this->GetCommunicationModule();

  vtkClientServerStream str;
  int i;

  // TODO revise
  for (i=0; i<numObjects; i++)
    {
    str << vtkClientServerStream::Invoke 
        << this->GetID(i) << "DoubleBufferOff"
        << vtkClientServerStream::End;
    }

  this->RendererProxy->SetVTKClassName("vtkRenderer");
  this->RendererProxy->CreateVTKObjects(numObjects);
  for (i=0; i<numObjects; i++)
    {
    str << vtkClientServerStream::Invoke 
        << this->GetID(i) << "AddRenderer" << this->RendererProxy->GetID(i)
        << vtkClientServerStream::End;
    }

  this->CameraProxy->SetVTKClassName("vtkCamera");
  this->CameraProxy->CreateVTKObjects(numObjects);
  for (i=0; i<numObjects; i++)
    {
    str << vtkClientServerStream::Invoke 
        << this->RendererProxy->GetID(i) 
        << "SetActiveCamera" 
        << this->CameraProxy->GetID(i)
        << vtkClientServerStream::End;
    }

  this->CompositeProxy->SetVTKClassName("vtkCompositeManager");
  this->CompositeProxy->CreateVTKObjects(numObjects);
  for (i=0; i<numObjects; i++)
    {
    str << vtkClientServerStream::Invoke 
        << this->CompositeProxy->GetID(i) 
        << "SetRenderWindow" 
        << this->GetID(i)
        << vtkClientServerStream::End;
    str << vtkClientServerStream::Invoke 
        << this->CompositeProxy->GetID(i) 
        << "InitializeRMIs" 
        << vtkClientServerStream::End;
    }

  cm->SendStreamToServers(&str, 
                          this->GetNumberOfServerIDs(),
                          this->GetServerIDs());

  str.Print(cout);
  str.Reset();

  this->WindowToImage->SetVTKClassName("vtkWindowToImageFilter");
  this->WindowToImage->CreateVTKObjects(1);
  
  if (numObjects > 0)
    {
    str << vtkClientServerStream::Invoke 
        << this->WindowToImage->GetID(0) 
        << "SetInput" 
        << this->GetID(0)
        << vtkClientServerStream::End;
    }
  
  this->ImageWriter->SetVTKClassName("vtkJPEGWriter");
  this->ImageWriter->CreateVTKObjects(1);
  
  str << vtkClientServerStream::Invoke 
      << this->WindowToImage->GetID(0) 
      << "GetOutput" 
      << vtkClientServerStream::End;
  
  str << vtkClientServerStream::Invoke 
      << this->ImageWriter->GetID(0) 
      << "SetInput" 
      << vtkClientServerStream::LastResult
      << vtkClientServerStream::End;

  cm->SendStreamToServers(&str, 
                          this->ImageWriter->GetNumberOfServerIDs(),
                          this->ImageWriter->GetServerIDs());
}

//---------------------------------------------------------------------------
void vtkSMDisplayWindowProxy::WriteImage(const char* filename)
{
  vtkClientServerStream str;

  str << vtkClientServerStream::Invoke 
      << this->WindowToImage->GetID(0) 
      << "Modified" 
      << vtkClientServerStream::End;

  str << vtkClientServerStream::Invoke 
      << this->ImageWriter->GetID(0) 
      << "SetFileName" 
      << filename
      << vtkClientServerStream::End;

  str << vtkClientServerStream::Invoke 
      << this->ImageWriter->GetID(0) 
      << "Write" 
      << vtkClientServerStream::End;

  vtkSMCommunicationModule* cm = this->GetCommunicationModule();
  cm->SendStreamToServers(&str, 
                          this->ImageWriter->GetNumberOfServerIDs(),
                          this->ImageWriter->GetServerIDs());
  str.Print(cout);
  str.Reset();

}

//---------------------------------------------------------------------------
void vtkSMDisplayWindowProxy::AddDisplay(vtkSMDisplayerProxy* display)
{
  this->CreateVTKObjects(1);

  vtkClientServerStream str;
  int numActors = display->GetActorProxy()->GetNumberOfIDs();
  for (int i=0; i<numActors; i++)
    {
    str << vtkClientServerStream::Invoke 
        << this->RendererProxy->GetID(0) 
        << "AddActor" 
        << display->GetActorProxy()->GetID(i)
        << vtkClientServerStream::End;
    }
  vtkSMCommunicationModule* cm = this->GetCommunicationModule();
  cm->SendStreamToServers(&str, 
                          this->GetNumberOfServerIDs(),
                          this->GetServerIDs());
  str.Print(cout);
  str.Reset();
}

//---------------------------------------------------------------------------
void vtkSMDisplayWindowProxy::StillRender()
{
  int numObjects = this->GetNumberOfIDs();
  vtkClientServerStream str;
  for (int i=0; i<numObjects; i++)
    {
    str << vtkClientServerStream::Invoke 
        << this->GetID(i) << "Render"
        << vtkClientServerStream::End;
    }
  vtkSMCommunicationModule* cm = this->GetCommunicationModule();
  // TODO This might not be the right thing to do if we are using
  // a composite manager. Then it should be enough to call Render
  // on the server manager
  int serverids = 0;
  cm->SendStreamToServers(&str, 
                          1,
                          &serverids);
//  cm->SendStreamToServers(&str, 
//                          this->GetNumberOfServerIDs(),
//                          this->GetServerIDs());
  str.Print(cout);
  str.Reset();
}

//---------------------------------------------------------------------------
void vtkSMDisplayWindowProxy::InteractiveRender()
{
  // TODO implement this
  this->StillRender();
}

//---------------------------------------------------------------------------
void vtkSMDisplayWindowProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
