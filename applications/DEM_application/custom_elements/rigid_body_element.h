// Created by: Salva Latorre, latorre@cimne.upc.edu

#if !defined KRATOS_RIGID_BODY_ELEMENT_H_INCLUDED
#define KRATOS_RIGID_BODY_ELEMENT_H_INCLUDED

// System includes
#include <string>
#include <iostream> 
#include <cmath>

// Project includes
#include "includes/define.h"
#include "includes/node.h"
#include "includes/element.h"
#include "geometries/geometry.h"
#include "includes/properties.h"
#include "includes/process_info.h"
#include "utilities/indexed_object.h"
#include "containers/weak_pointer_vector.h"
#include "includes/constitutive_law.h"
#include "includes/condition.h"
#include "custom_utilities/create_and_destroy.h"
#include "utilities/quaternion.h"
#include "custom_conditions/RigidFace.h"
#include "../custom_strategies/schemes/dem_integration_scheme.h"


namespace Kratos {
    
    class RigidBodyElement3D : public Element {
        
    public:
        /// Pointer definition of RigidBodyElement3D
        KRATOS_CLASS_POINTER_DEFINITION(RigidBodyElement3D);
       
        RigidBodyElement3D();
        RigidBodyElement3D(IndexType NewId, GeometryType::Pointer pGeometry);
        RigidBodyElement3D(IndexType NewId, NodesArrayType const& ThisNodes);
        RigidBodyElement3D(IndexType NewId, GeometryType::Pointer pGeometry,  PropertiesType::Pointer pProperties);

        Element::Pointer Create(IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const;      

        /// Destructor
        virtual ~RigidBodyElement3D();
      
        using Element::Initialize;
        virtual void Initialize(ProcessInfo& r_process_info, ModelPart& rigid_body_element_sub_model_part);
        virtual void SetIntegrationScheme(DEMIntegrationScheme::Pointer& integration_scheme);
        virtual void InitializeSolutionStep(ProcessInfo& r_process_info){};
        virtual void FinalizeSolutionStep(ProcessInfo& r_process_info){};
        virtual void CustomInitialize(ProcessInfo& r_process_info);
        virtual void SetOrientation(const Quaternion<double> Orientation);
        virtual void UpdatePositionOfNodes();
        virtual void UpdateLinearDisplacementAndVelocityOfNodes();
        virtual void GetRigidBodyElementsForce(const array_1d<double,3>& gravity);
        virtual void CollectForcesAndTorquesFromNodes();
        virtual void ComputeAdditionalForces(const array_1d<double,3>& gravity);
        virtual void Calculate(const Variable<double>& rVariable, double& Output, const ProcessInfo& r_process_info);
        virtual void SetInitialConditionsToNodes(const array_1d<double,3>& velocity);
        
        virtual void Move(const double delta_t, const bool rotation_option, const double force_reduction_factor, const int StepFlag);
        virtual DEMIntegrationScheme& GetIntegrationScheme() { return *mpIntegrationScheme; }
   
        double GetSqrtOfRealMass();

        virtual std::string Info() const
        {
	    std::stringstream buffer;
	    buffer << "Discrete Element #" << Id();
	    return buffer.str();
        }
      
        /// Print information about this object.
        virtual void PrintInfo(std::ostream& rOStream) const
        {
	    rOStream << "Discrete Element #" << Id();
        }
      
        /// Print object's data.
        virtual void PrintData(std::ostream& rOStream) const
        {
	    //mpGeometry->PrintData(rOStream);
        }
        
        std::vector<array_1d<double, 3> > mListOfCoordinates;
        std::vector<Node<3>::Pointer > mListOfNodes;
        DEMIntegrationScheme* mpIntegrationScheme;
 
    //protected:

        std::vector<RigidFace3D*> mListOfRigidFaces;
        array_1d<double,3> mInertias;                                
        double mMass;
        std::vector<SphericParticle*> mListOfSphericParticles; 

      
    private:
       
        friend class Serializer;

        virtual void save(Serializer& rSerializer) const
        {
            KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Element);
        }

        virtual void load(Serializer& rSerializer)
        {
            KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Element);
        }

    }; // Class RigidBodyElement3D
   
    /// input stream function
    inline std::istream& operator >> (std::istream& rIStream, RigidBodyElement3D& rThis);

    /// output stream function
    inline std::ostream& operator << (std::ostream& rOStream, const RigidBodyElement3D& rThis)
    {
        rThis.PrintInfo(rOStream);
        rOStream << std::endl;
        rThis.PrintData(rOStream);

        return rOStream;
    }
 
} // namespace Kratos

#endif // KRATOS_RIGID_BODY_ELEMENT_H_INCLUDED  defined
