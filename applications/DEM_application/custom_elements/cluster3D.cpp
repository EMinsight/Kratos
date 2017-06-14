// Last Modified by: Salva $

// System includes
#include <string>
#include <iostream>
#include <stdlib.h>

// Project includes
#include "cluster3D.h"
#include "custom_utilities/GeometryFunctions.h"
#include "custom_utilities/AuxiliaryFunctions.h"
#include "DEM_application_variables.h"
#include "includes/kratos_flags.h"
#include "includes/variables.h"
#include "custom_elements/spheric_continuum_particle.h"

namespace Kratos {

    Cluster3D::Cluster3D() : RigidBodyElement3D() {}
            
    Cluster3D::Cluster3D(IndexType NewId, GeometryType::Pointer pGeometry)
    : RigidBodyElement3D(NewId, pGeometry) {
        mpIntegrationScheme = NULL;
    }
      
    Cluster3D::Cluster3D(IndexType NewId, GeometryType::Pointer pGeometry, PropertiesType::Pointer pProperties)
    : RigidBodyElement3D(NewId, pGeometry, pProperties) {
        mpIntegrationScheme = NULL;
    }
      
    Cluster3D::Cluster3D(IndexType NewId, NodesArrayType const& ThisNodes)
    : RigidBodyElement3D(NewId, ThisNodes) {
        mpIntegrationScheme = NULL;
    }
    
    Element::Pointer Cluster3D::Create(IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const {
        return Element::Pointer(new Cluster3D(NewId, GetGeometry().Create(ThisNodes), pProperties));
    }      
    
    // Destructor
    Cluster3D::~Cluster3D() {

        if (GetProperties()[BREAKABLE_CLUSTER]) {
            for (unsigned int i = 0; i < mListOfCoordinates.size(); i++) {
                mListOfSphericParticles[i]->Set(DEMFlags::BELONGS_TO_A_CLUSTER, false);
                mListOfSphericParticles[i]->GetGeometry()[0].Set(DEMFlags::BELONGS_TO_A_CLUSTER, false);
            }  
            GetGeometry()[0].Set(TO_ERASE, true); 
        }
        else{
            for (unsigned int i = 0; i < mListOfCoordinates.size(); i++) {
                mListOfSphericParticles[i]->Set(DEMFlags::BELONGS_TO_A_CLUSTER, false);
                mListOfSphericParticles[i]->GetGeometry()[0].Set(DEMFlags::BELONGS_TO_A_CLUSTER, false);
                mListOfSphericParticles[i]->Set(TO_ERASE, true);                        
            }    
        }

        mListOfSphericParticles.clear();
        mListOfCoordinates.clear();  
        mListOfRadii.clear();  
        
        if (mpIntegrationScheme != NULL) {
            delete mpIntegrationScheme;
        }        

    }

    void Cluster3D::Initialize(ProcessInfo& r_process_info) {
        
        if (GetGeometry()[0].GetDof(VELOCITY_X).IsFixed())          GetGeometry()[0].Set(DEMFlags::FIXED_VEL_X, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_VEL_X, false);
        if (GetGeometry()[0].GetDof(VELOCITY_Y).IsFixed())          GetGeometry()[0].Set(DEMFlags::FIXED_VEL_Y, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_VEL_Y, false);
        if (GetGeometry()[0].GetDof(VELOCITY_Z).IsFixed())          GetGeometry()[0].Set(DEMFlags::FIXED_VEL_Z, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_VEL_Z, false);
        if (GetGeometry()[0].GetDof(ANGULAR_VELOCITY_X).IsFixed())  GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_X, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_X, false);
        if (GetGeometry()[0].GetDof(ANGULAR_VELOCITY_Y).IsFixed())  GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_Y, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_Y, false);
        if (GetGeometry()[0].GetDof(ANGULAR_VELOCITY_Z).IsFixed())  GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_Z, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_Z, false);

        CustomInitialize(r_process_info);
        
        DEMIntegrationScheme::Pointer& integration_scheme = GetProperties()[DEM_INTEGRATION_SCHEME_POINTER];
        SetIntegrationScheme(integration_scheme);
    }

    void Cluster3D::CustomInitialize(ProcessInfo& r_process_info) {
        
        const double cl = GetGeometry()[0].FastGetSolutionStepValue(CHARACTERISTIC_LENGTH);
        const ClusterInformation& cl_info = GetProperties()[CLUSTER_INFORMATION];
        //std::string& name = cl_info.mName;
        const double reference_size = cl_info.mSize;
        const double reference_volume = cl_info.mVolume;
        const std::vector<double>& reference_list_of_radii = cl_info.mListOfRadii;
        const std::vector<array_1d<double,3> >& reference_list_of_coordinates = cl_info.mListOfCoordinates;
        const array_1d<double,3>& reference_inertias = cl_info.mInertias;
        
        const unsigned int number_of_spheres = reference_list_of_radii.size();
        
        mListOfRadii.resize(number_of_spheres);
        mListOfCoordinates.resize(number_of_spheres);
        mListOfSphericParticles.resize(number_of_spheres);
        
        const double scaling_factor = cl / reference_size;
        
        for (int i = 0; i < (int)number_of_spheres; i++) {
            mListOfRadii[i] = scaling_factor * reference_list_of_radii[i];
            mListOfCoordinates[i][0] = scaling_factor * reference_list_of_coordinates[i][0];
            mListOfCoordinates[i][1] = scaling_factor * reference_list_of_coordinates[i][1];
            mListOfCoordinates[i][2] = scaling_factor * reference_list_of_coordinates[i][2];
        }
                        
        const double particle_density = this->SlowGetDensity();         
        const double cluster_volume = reference_volume * scaling_factor*scaling_factor*scaling_factor;        
        const double cluster_mass = particle_density * cluster_volume;
        
        GetGeometry()[0].FastGetSolutionStepValue(NODAL_MASS) = cluster_mass;
        GetGeometry()[0].FastGetSolutionStepValue(CLUSTER_VOLUME) = cluster_volume;
        GetGeometry()[0].FastGetSolutionStepValue(PARTICLE_MATERIAL) = this->SlowGetParticleMaterial();
        
        const double squared_scaling_factor_times_density = scaling_factor * scaling_factor * particle_density;
        GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[0] = reference_inertias[0] * cluster_volume * squared_scaling_factor_times_density;
        GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[1] = reference_inertias[1] * cluster_volume * squared_scaling_factor_times_density;
        GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[2] = reference_inertias[2] * cluster_volume * squared_scaling_factor_times_density;
        
        array_1d<double, 3> base_principal_moments_of_inertia = GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA);  
        
        Quaternion<double>& Orientation = GetGeometry()[0].FastGetSolutionStepValue(ORIENTATION);
                        
        Orientation.normalize();

        array_1d<double, 3> angular_velocity = GetGeometry()[0].FastGetSolutionStepValue(ANGULAR_VELOCITY);
        
        array_1d<double, 3> angular_momentum;
        double LocalTensor[3][3];
        double GlobalTensor[3][3];
        GeometryFunctions::ConstructLocalTensor(base_principal_moments_of_inertia, LocalTensor);
        GeometryFunctions::QuaternionTensorLocal2Global(Orientation, LocalTensor, GlobalTensor);                   
        GeometryFunctions::ProductMatrix3X3Vector3X1(GlobalTensor, angular_velocity, angular_momentum);
        noalias(this->GetGeometry()[0].FastGetSolutionStepValue(ANGULAR_MOMENTUM)) = angular_momentum;

        array_1d<double, 3> local_angular_velocity;
        GeometryFunctions::QuaternionVectorGlobal2Local(Orientation, angular_velocity, local_angular_velocity);
        noalias(this->GetGeometry()[0].FastGetSolutionStepValue(LOCAL_ANGULAR_VELOCITY)) = local_angular_velocity;
    }

    void Cluster3D::CreateParticles(ParticleCreatorDestructor* p_creator_destructor, ModelPart& dem_model_part, PropertiesProxy* p_fast_properties, const bool continuum_strategy) {        
        
        KRATOS_TRY 
        
        int cluster_id = (int)this->Id();
               
        unsigned int max_Id = 0;        
        unsigned int* p_max_Id = p_creator_destructor->pGetCurrentMaxNodeId();  //must have been found
          
        std::string ElementNameString;
        bool breakable = false;
        if (GetProperties()[BREAKABLE_CLUSTER]) breakable = true;
        
        if (continuum_strategy) ElementNameString= "SphericContinuumParticle3D";
        else ElementNameString= "SphericParticle3D";
        
        if (!continuum_strategy && breakable) KRATOS_THROW_ERROR(std::runtime_error,"Breakable cluster elements are being used inside a non-deformable strategy. The program will now stop.","")

        //We now create a spheric particle and keep it as a reference to an Element
        const Element& r_reference_element = KratosComponents<Element>::Get(ElementNameString);
        
        Node<3>& central_node = GetGeometry()[0]; //CENTRAL NODE OF THE CLUSTER
          
        Quaternion<double>& Orientation = central_node.FastGetSolutionStepValue(ORIENTATION);

        const double mass = central_node.FastGetSolutionStepValue(NODAL_MASS);
        array_1d<double, 3> coordinates_of_sphere;
        array_1d<double, 3> global_relative_coordinates;
        double radius_of_sphere;
                
        for (unsigned int i = 0; i < mListOfCoordinates.size(); i++) {

            GeometryFunctions::QuaternionVectorLocal2Global(Orientation, mListOfCoordinates[i], global_relative_coordinates);
            coordinates_of_sphere[0]= central_node.Coordinates()[0] + global_relative_coordinates[0];
            coordinates_of_sphere[1]= central_node.Coordinates()[1] + global_relative_coordinates[1];
            coordinates_of_sphere[2]= central_node.Coordinates()[2] + global_relative_coordinates[2];    
            
            radius_of_sphere = mListOfRadii[i];   
            #pragma omp critical
            {
                (*p_max_Id)++;
                max_Id = *p_max_Id;
            }
             
            Kratos::SphericParticle* new_sphere;
            if (!breakable) {
                new_sphere = p_creator_destructor->SphereCreatorForClusters(dem_model_part, 
                                                                            max_Id, 
                                                                            radius_of_sphere, 
                                                                            coordinates_of_sphere, 
                                                                            mass,
                                                                            this->pGetProperties(), 
                                                                            r_reference_element,
                                                                            cluster_id,
                                                                            p_fast_properties);
            }
            else{
                new_sphere = p_creator_destructor->SphereCreatorForBreakableClusters(dem_model_part, 
                                                                                    max_Id, 
                                                                                    radius_of_sphere, 
                                                                                    coordinates_of_sphere, 
                                                                                    this->pGetProperties(), 
                                                                                    r_reference_element,
                                                                                    cluster_id,
                                                                                    p_fast_properties);
            }
                        
            mListOfSphericParticles[i] = new_sphere;                 
        }

        KRATOS_CATCH("")
    }
    
    void Cluster3D::CreateContinuumConstitutiveLaws() {        
        for (unsigned int i=0; i<mListOfCoordinates.size(); i++) {  
            SphericContinuumParticle* p_continuum_spheric_particle = dynamic_cast<SphericContinuumParticle*> (mListOfSphericParticles[i]);            
            p_continuum_spheric_particle->CreateContinuumConstitutiveLaws();
        }         
    }

    void Cluster3D::SetContinuumGroupToBreakableClusterSpheres(const int Id) {
        for (unsigned int i=0; i<mListOfSphericParticles.size(); i++) {
            SphericContinuumParticle* p_cont_part = dynamic_cast<SphericContinuumParticle*> (mListOfSphericParticles[i]);
            p_cont_part->mContinuumGroup = Id;
        }            
    }
    
    void Cluster3D::SetInitialConditionsToSpheres(const array_1d<double,3>& velocity) {
        for (unsigned int i=0; i<mListOfSphericParticles.size(); i++) {
            mListOfSphericParticles[i]->GetGeometry()[0].FastGetSolutionStepValue(VELOCITY) = velocity;
        } 
    }
    
    double Cluster3D::SlowGetDensity() { return GetProperties()[PARTICLE_DENSITY];}
    
    int Cluster3D::SlowGetParticleMaterial() { return GetProperties()[PARTICLE_MATERIAL];}
    
    void Cluster3D::SetInitialNeighbours(const double search_tolerance) {

        if (!mListOfSphericParticles.size()) return;
        /*for (unsigned int i=0; i<mListOfSphericParticles.size(); i++) {
            SphericContinuumParticle* p_continuum_particle_i = dynamic_cast<SphericContinuumParticle*> (mListOfSphericParticles[i]);
            //p_continuum_particle_i->mContinuumInitialNeighborsSize = 0;
            //p_continuum_particle_i->mInitialNeighborsSize = 0;
        }*/
        
        for (unsigned int i = 0; i < mListOfSphericParticles.size() - 1; i++) {
            SphericContinuumParticle* p_continuum_particle_i = dynamic_cast<SphericContinuumParticle*> (mListOfSphericParticles[i]);            
            
            if (mListOfSphericParticles.size() <= 1) break;
            array_1d<double, 3> zero_vector(3, 0.0);
            
            for (unsigned int j = i + 1; j < mListOfSphericParticles.size(); j++) {
                SphericContinuumParticle* p_continuum_particle_j = dynamic_cast<SphericContinuumParticle*> (mListOfSphericParticles[j]);
                
                array_1d<double, 3> other_to_me_vect;
                noalias(other_to_me_vect)= p_continuum_particle_i->GetGeometry()[0].Coordinates() - p_continuum_particle_j->GetGeometry()[0].Coordinates();

                double distance = DEM_MODULUS_3(other_to_me_vect);
                double radius_sum = p_continuum_particle_i->GetRadius() + p_continuum_particle_j->GetRadius();
                
                if (distance < radius_sum + search_tolerance) {                                                        
                    double initial_delta = radius_sum - distance;
                    
                    p_continuum_particle_i->mNeighbourElements.push_back(p_continuum_particle_j);
                    p_continuum_particle_i->mIniNeighbourIds.push_back(p_continuum_particle_j->Id());
                    p_continuum_particle_i->mIniNeighbourDelta.push_back(initial_delta);
                    p_continuum_particle_i->mIniNeighbourFailureId.push_back(0);
                    p_continuum_particle_i->mContinuumInitialNeighborsSize++;
                    p_continuum_particle_i->mInitialNeighborsSize++;
                    p_continuum_particle_i->mNeighbourElasticContactForces.push_back(zero_vector);
                    p_continuum_particle_i->mNeighbourElasticExtraContactForces.push_back(zero_vector);
                    
                    p_continuum_particle_j->mNeighbourElements.push_back(p_continuum_particle_i);
                    p_continuum_particle_j->mIniNeighbourIds.push_back(p_continuum_particle_i->Id());
                    p_continuum_particle_j->mIniNeighbourDelta.push_back(initial_delta);
                    p_continuum_particle_j->mIniNeighbourFailureId.push_back(0);
                    p_continuum_particle_j->mContinuumInitialNeighborsSize++;
                    p_continuum_particle_j->mInitialNeighborsSize++; 
                    p_continuum_particle_j->mNeighbourElasticContactForces.push_back(zero_vector);
                    p_continuum_particle_j->mNeighbourElasticExtraContactForces.push_back(zero_vector);
                }
            }   
        }               
    }
    
    void Cluster3D::Move(const double delta_t, const bool rotation_option, const double force_reduction_factor, const int StepFlag ) {
        GetIntegrationScheme().MoveCluster(this, GetGeometry()[0], delta_t, rotation_option, force_reduction_factor, StepFlag);            
    }
    
}  // namespace Kratos
