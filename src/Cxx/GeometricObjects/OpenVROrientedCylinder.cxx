#include <vtkActor.h>
#include <vtkCylinderSource.h>
#include <vtkMath.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkNamedColors.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <array>
#define USER_MATRIX

int main(int, char *[])
{
  vtkSmartPointer<vtkNamedColors> colors =
  	vtkSmartPointer<vtkNamedColors>::New();
  
  // Set the background color.
  std::array<unsigned char, 4> bkg{ { 26, 51, 77, 255 } };
  colors->SetColor("BkgColor", bkg.data());
  
  
  // Create a cylinder.
  // Cylinder height vector is (0,1,0).
  // Cylinder center is in the middle of the cylinder
  vtkSmartPointer<vtkCylinderSource> cylinderSource =
  	vtkSmartPointer<vtkCylinderSource>::New();
  cylinderSource->SetResolution(15);
  
  // Generate a random start and end point
  double startPoint[3];
  double endPoint[3];
  vtkSmartPointer<vtkMinimalStandardRandomSequence> rng =
  	vtkSmartPointer<vtkMinimalStandardRandomSequence>::New();
  rng->SetSeed(8775070); // For testing.
  for (auto i = 0; i < 3; ++i)
  {
  	rng->Next();
  	startPoint[i] = rng->GetRangeValue(-10, 10);
  	rng->Next();
  	endPoint[i] = rng->GetRangeValue(-10, 10);
  }
  
  // Compute a basis
  double normalizedX[3];
  double normalizedY[3];
  double normalizedZ[3];
  
  // The X axis is a vector from start to end
  vtkMath::Subtract(endPoint, startPoint, normalizedX);
  double length = vtkMath::Norm(normalizedX);
  vtkMath::Normalize(normalizedX);
  
  // The Z axis is an arbitrary vector cross X
  double arbitrary[3];
  for (auto i = 0; i < 3; ++i)
  {
  	rng->Next();
  	arbitrary[i] = rng->GetRangeValue(-10, 10);
  }
  vtkMath::Cross(normalizedX, arbitrary, normalizedZ);
  vtkMath::Normalize(normalizedZ);
  
  // The Y axis is Z cross X
  vtkMath::Cross(normalizedZ, normalizedX, normalizedY);
  vtkSmartPointer<vtkMatrix4x4> matrix =
  	vtkSmartPointer<vtkMatrix4x4>::New();
  
  // Create the direction cosine matrix
  matrix->Identity();
  for (unsigned int i = 0; i < 3; i++)
  {
  	matrix->SetElement(i, 0, normalizedX[i]);
  	matrix->SetElement(i, 1, normalizedY[i]);
  	matrix->SetElement(i, 2, normalizedZ[i]);
  }
  
  // Apply the transforms
  vtkSmartPointer<vtkTransform> transform =
  	vtkSmartPointer<vtkTransform>::New();
  transform->Translate(startPoint);   // translate to starting point
  transform->Concatenate(matrix);     // apply direction cosines
  transform->RotateZ(-90.0);          // align cylinder to x axis
  transform->Scale(1.0, length, 1.0); // scale along the height vector
  transform->Translate(0, .5, 0);     // translate to start of cylinder
  
  									// Transform the polydata
  vtkSmartPointer<vtkTransformPolyDataFilter> transformPD =
  	vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformPD->SetTransform(transform);
  transformPD->SetInputConnection(cylinderSource->GetOutputPort());
  
  //Create a mapper and actor for the cylinder
  vtkSmartPointer<vtkPolyDataMapper> mapper =
  	vtkSmartPointer<vtkPolyDataMapper>::New();
  vtkSmartPointer<vtkActor> actor =
  	vtkSmartPointer<vtkActor>::New();
#ifdef USER_MATRIX
  mapper->SetInputConnection(cylinderSource->GetOutputPort());
  actor->SetUserMatrix(transform->GetMatrix());
#else
  mapper->SetInputConnection(transformPD->GetOutputPort());
#endif
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d("Cyan").GetData());
  
  // Create spheres for start and end point
  vtkSmartPointer<vtkSphereSource> sphereStartSource =
  	vtkSmartPointer<vtkSphereSource>::New();
  sphereStartSource->SetCenter(startPoint);
  sphereStartSource->SetRadius(0.8);
  vtkSmartPointer<vtkPolyDataMapper> sphereStartMapper =
  	vtkSmartPointer<vtkPolyDataMapper>::New();
  sphereStartMapper->SetInputConnection(sphereStartSource->GetOutputPort());
  vtkSmartPointer<vtkActor> sphereStart =
  	vtkSmartPointer<vtkActor>::New();
  sphereStart->SetMapper(sphereStartMapper);
  sphereStart->GetProperty()->SetColor(colors->GetColor3d("Yellow").GetData());
  
  vtkSmartPointer<vtkSphereSource> sphereEndSource =
  	vtkSmartPointer<vtkSphereSource>::New();
  sphereEndSource->SetCenter(endPoint);
  sphereEndSource->SetRadius(0.8);
  vtkSmartPointer<vtkPolyDataMapper> sphereEndMapper =
  	vtkSmartPointer<vtkPolyDataMapper>::New();
  sphereEndMapper->SetInputConnection(sphereEndSource->GetOutputPort());
  vtkSmartPointer<vtkActor> sphereEnd =
  	vtkSmartPointer<vtkActor>::New();
  sphereEnd->SetMapper(sphereEndMapper);
  sphereEnd->GetProperty()->SetColor(colors->GetColor3d("Magenta").GetData());
  
  //Create a renderer, render window, and interactor
  vtkSmartPointer<vtkOpenVRRenderer> renderer =
  	vtkSmartPointer<vtkOpenVRRenderer>::New();
  vtkSmartPointer<vtkOpenVRRenderWindow> renderWindow =
  	vtkSmartPointer<vtkOpenVRRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  vtkSmartPointer<vtkOpenVRRenderWindowInteractor> renderWindowInteractor =
  	vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);
  
  //Add the actor to the scene
  renderer->AddActor(actor);
  renderer->AddActor(sphereStart);
  renderer->AddActor(sphereEnd);
  renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
  
  //Render and interact
  renderWindow->Render();
  renderWindowInteractor->Start();
  
  return EXIT_SUCCESS;
}
