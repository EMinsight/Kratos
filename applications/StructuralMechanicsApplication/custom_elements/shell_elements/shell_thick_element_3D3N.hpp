// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:         BSD License
//                   license: StructuralMechanicsApplication/license.txt
//
//  Main authors:    Peter Wilson
//       contact:    A.Winterstein[at]tum.de
//

#pragma once


// System includes
#include <type_traits>

// External includes

// Project includes
#include "base_shell_element.h"
#include "custom_utilities/shellt3_corotational_coordinate_transformation.hpp"
#include "custom_utilities/shellt3_local_coordinate_system.hpp"

namespace Kratos
{
///@name Kratos Globals
///@{
///@}

///@name Type Definitions
///@{
///@}

///@name  Enum's
///@{
///@}

///@name  Functions
///@{
///@}

///@name Kratos Classes
///@{

/** \brief ShellThickElement3D3N
*
* This element represents a 3-node Shell element
* based on the Discrete Shear Gap theory (DSG) by Bletzinger.
* This element is formulated for small strains,
* but can be used in Geometrically nonlinear problems
* involving large displacements and rotations
* using a Corotational Coordinate Transformation.
* Material nonlinearity is handled by means of the cross section object.
*/

/*
Shell formulation reference:
1.    Bletzinger, K.U., Bischoff, M. and Ramm, E., 2000. A unified approach for
    shear-locking-free triangular and rectangular shell finite elements.
    Computers & Structures, 75(3), pp.321-334.
2.    Rama, G.,  Marinkovic, D.,  Zehn, M., 2016. Efficient co-rotational
    3-node shell element.
    American Journal of Engineering and Applied Sciences, Volume 9, Issue 2,
    Pages 420-431.
*/

template <ShellKinematics TKinematics>
class KRATOS_API(STRUCTURAL_MECHANICS_APPLICATION) ShellThickElement3D3N : public
    BaseShellElement<typename std::conditional<TKinematics==ShellKinematics::NONLINEAR_COROTATIONAL,
        ShellT3_CorotationalCoordinateTransformation,
        ShellT3_CoordinateTransformation>::type>
{
public:

    ///@name Type Definitions
    ///@{

    KRATOS_CLASS_INTRUSIVE_POINTER_DEFINITION(ShellThickElement3D3N);

    using BaseType = BaseShellElement<typename std::conditional<TKinematics==ShellKinematics::NONLINEAR_COROTATIONAL,
        ShellT3_CorotationalCoordinateTransformation,
        ShellT3_CoordinateTransformation>::type>;

    typedef Quaternion<double> QuaternionType;

    using GeometryType = Element::GeometryType;

    using PropertiesType = Element::PropertiesType;

    using NodesArrayType = Element::NodesArrayType;

    using MatrixType = Element::MatrixType;

    using VectorType = Element::VectorType;

    using SizeType = Element::SizeType;

    using Element::GetGeometry;

    using Element::GetProperties;

    using Vector3Type = typename BaseType::Vector3Type;

    using CoordinateTransformationPointerType = typename BaseType::CoordinateTransformationPointerType;

    ///@}

    ///@name Classes
    ///@{

    ///@}

    ///@name Life Cycle
    ///@{

    ShellThickElement3D3N(IndexType NewId,
                          GeometryType::Pointer pGeometry);

    ShellThickElement3D3N(IndexType NewId,
                          GeometryType::Pointer pGeometry,
                          PropertiesType::Pointer pProperties);

    ~ShellThickElement3D3N() override = default;

    ///@}

    ///@name Operations
    ///@{

    // Basic

    /**
     * @brief Creates a new element
     * @param NewId The Id of the new created element
     * @param pGeom The pointer to the geometry of the element
     * @param pProperties The pointer to property
     * @return The pointer to the created element
     */
    Element::Pointer Create(
        IndexType NewId,
        GeometryType::Pointer pGeom,
        PropertiesType::Pointer pProperties
    ) const override;

    /**
     * @brief Creates a new element
     * @param NewId The Id of the new created element
     * @param ThisNodes The array containing nodes
     * @param pProperties The pointer to property
     * @return The pointer to the created element
     */
    Element::Pointer Create(
        IndexType NewId,
        NodesArrayType const& ThisNodes,
        PropertiesType::Pointer pProperties
    ) const override;

    // More results calculation on integration points to interface with python

    using BaseType::CalculateOnIntegrationPoints;

    void CalculateOnIntegrationPoints(const Variable<double>& rVariable,
                                      std::vector<double>& rOutput, const ProcessInfo& rCurrentProcessInfo) override;

    void CalculateOnIntegrationPoints(const Variable<Matrix>& rVariable,
                                      std::vector<Matrix>& rOutput, const ProcessInfo& rCurrentProcessInfo) override;

    /**
    * This method provides the place to perform checks on the completeness of the input
    * and the compatibility with the problem options as well as the contitutive laws selected
    * It is designed to be called only once (or anyway, not often) typically at the beginning
    * of the calculations, so to verify that nothing is missing from the input
    * or that no common error is found.
    * @param rCurrentProcessInfo
    * this method is: MANDATORY
    */
    int Check(const ProcessInfo& rCurrentProcessInfo) const override;

    ///@}

    ///@name Public specialized Access - Temporary
    ///@{

    ///@}

protected:

    ///@name Protected Lyfe Cycle
    ///@{

    /**
    * Protected empty constructor
    */
    ShellThickElement3D3N() : BaseType()
    {
    }

    ///@}

private:

    ///@name Private Classes
    ///@{

    class CalculationData
    {

    public:

        // ---------------------------------------
        // calculation-constant data
        // ----------------------------------------
        // these data are allocated and constructed
        // at the beginning of the calculation

        ShellT3_LocalCoordinateSystem LCS0; /*!< reference coordinate system */
        ShellT3_LocalCoordinateSystem LCS;  /*!< current coordinate system */

        double dA;
        double hMean;
        double TotalArea;

        std::vector< array_1d<double, 3> > gpLocations;

        MatrixType dNxy = ZeroMatrix(3, 2);  /*!< shape function cartesian derivatives */
        VectorType N = ZeroVector(3); /*!< shape function vector at the current integration point */

        VectorType globalDisplacements = ZeroVector(18); /*!< global displacement vector */
        VectorType localDisplacements = ZeroVector(18);  /*!< local displacement vector */

        bool CalculateRHS; /*!< flag for the calculation of the right-hand-side vector */
        bool CalculateLHS; /*!< flag for the calculation of the left-hand-side vector */

        const bool parabolic_composite_transverse_shear_strains = false;

        // ---------------------------------------
        // Testing flags
        // ---------------------------------------
        // These should all be FALSE unless you are testing, or
        // investigating the effects of element enhancements!

        const bool basicTriCST = false;    // bool to use basic CST
        // displacement-based shear formulation. This should be FALSE unless
        // you are testing

        const bool ignore_shear_stabilization = false; // bool to
        // ignore stabilizing the transverse shear part of the material
        // matrix. This should be false unless you are testing

        const bool smoothedDSG = false; // bool to use smoothed DSG
        // formulation according to [Nguyen-Thoi et al., 2013].
        // This should be false unless you are testing

        const bool specialDSGc3 = false; // bool to use experimental
        // DSGc3 formulation not yet complete.
        // This should be false unless you are testing

        // ---------------------------------------
        // calculation-variable data
        // ---------------------------------------
        // these data are updated during the
        // calculations

        SizeType gpIndex;

        // ---------------------------------------
        // calculation-variable data
        // ---------------------------------------
        // these data are updated during the
        // calculations, but they are allocated
        // only once(the first time they are used)
        // to avoid useless re-allocations

        MatrixType B = ZeroMatrix(8, 18);   /*!< total strain-displacement matrix at the current integration point */

        double h_e;        /*!< longest edge of triangle */
        double alpha = 0.1;    // modifier of shear material matrix stabilization parameter
        // refer Lyly(1993)
        double shearStabilisation;

        Matrix D = ZeroMatrix(8, 8);        /*!< section constitutive matrix at the current integration point */

        VectorType generalizedStrains = ZeroVector(8);  /*!< generalized strain vector at the current integration point */

        VectorType generalizedStresses = ZeroVector(8); /*!< generalized stress vector at the current integration point */

        ShellCrossSection::SectionParameters SectionParameters; /*!< parameters for cross section calculations */

        std::vector<VectorType> rlaminateStrains;

        std::vector<VectorType> rlaminateStresses;

    public:

        const ProcessInfo& CurrentProcessInfo;

    public:

        CalculationData(const CoordinateTransformationPointerType& pCoordinateTransformation,
                        const ProcessInfo& rCurrentProcessInfo);

    };

    ///@}

    ///@name Private Operations
    ///@{
    void CalculateStressesFromForceResultants
    (VectorType& rstresses,
     const double& rthickness);

    void CalculateLaminaStrains(CalculationData& data);

    void CalculateLaminaStresses(CalculationData& data);

    double CalculateTsaiWuPlaneStress(const CalculationData& data, const Matrix& rLamina_Strengths, const unsigned int& rCurrent_Ply);

    void CalculateVonMisesStress(const CalculationData& data, const Variable<double>& rVariable, double& rVon_Mises_Result);

    void CalculateShellElementEnergy(const CalculationData& data, const Variable<double>& rVariable, double& rEnergy_Result);

    void CheckGeneralizedStressOrStrainOutput(const Variable<Matrix>& rVariable, int& iJob, bool& bGlobal);

    void CalculateSectionResponse(CalculationData& data);

    void InitializeCalculationData(CalculationData& data);

    void CalculateDSGc3Contribution(CalculationData& data, MatrixType& rLeftHandSideMatrix);

    void CalculateSmoothedDSGBMatrix(CalculationData& data);

    void CalculateDSGShearBMatrix(Matrix& shearBMatrix, const double& a, const double& b, const double& c, const double& d, const double& A);

    void AddBodyForces(CalculationData& data, VectorType& rRightHandSideVector);

    void CalculateAll(MatrixType& rLeftHandSideMatrix,
                      VectorType& rRightHandSideVector,
                      const ProcessInfo& rCurrentProcessInfo,
                      const bool CalculateStiffnessMatrixFlag,
                      const bool CalculateResidualVectorFlag) override;

    bool TryCalculateOnIntegrationPoints_GeneralizedStrainsOrStresses(const Variable<Matrix>& rVariable,
            std::vector<Matrix>& rValues,
            const ProcessInfo& rCurrentProcessInfo);

    /**
    * Returns the behavior of this shell (thin/thick)
    * @return the shell behavior
    */
    ShellCrossSection::SectionBehaviorType GetSectionBehavior() const override;

    ///@}

    ///@name Static Member Variables
    ///@{
    ///@}

    ///@name Member Variables
    ///@{

    ///@}

    ///@name Serialization
    ///@{

    friend class Serializer;

    void save(Serializer& rSerializer) const override;

    void load(Serializer& rSerializer) override;

    ///@}

    ///@name Private  Access
    ///@{
    ///@}

    ///@name Private Inquiry
    ///@{
    ///@}

    ///@name Un accessible methods
    ///@{
    ///@}

};

}
