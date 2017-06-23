from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7
#import kratos core and applications
import KratosMultiphysics
import KratosMultiphysics.ExternalSolversApplication as ExternalSolversApplication
import KratosMultiphysics.StructuralMechanicsApplication as StructuralMechanicsApplication

# Check that KratosMultiphysics was imported in the main script
KratosMultiphysics.CheckForPreviousImport()

import structural_mechanics_solver

def CreateSolver(main_model_part, custom_settings):
    return EigenSolver(main_model_part, custom_settings)

class EigenSolver(structural_mechanics_solver.MechanicalSolver):

    def __init__(self, main_model_part, custom_settings): 
        
        self.main_model_part = main_model_part
        
        ##settings string in json format
        default_settings = KratosMultiphysics.Parameters("""
        {
            "solver_type": "structural_mechanics_eigensolver",
            "echo_level": 0,
            "buffer_size": 1,
            "solution_type": "Dynamic",
            "analysis_type": "Linear",
            "model_import_settings": {
                "input_type": "mdpa",
                "input_filename": "unknown_name",
                "input_file_label": 0
            },
            "material_import_settings" :{
                "materials_filename": ""
            },
            "rotation_dofs": false,
            "pressure_dofs": false,
            "eigensolver_settings":{
                "solver_type": "FEAST"
            },
            "problem_domain_sub_model_part_list": ["solid_model_part"],
            "processes_sub_model_part_list": [""]
        }
        """)
        
        ##overwrite the default settings with user-provided parameters 
        self.settings = custom_settings
        self.settings.ValidateAndAssignDefaults(default_settings)
        
        # eigensolver_settings are validated/assigned in the linear_solver
        print("Construction of Eigensolver finished")

    def Initialize(self):
        self.compute_model_part = self.GetComputeModelPart()

        self.eigensolver_settings = self.settings["eigensolver_settings"] 
        if self.eigensolver_settings["solver_type"].GetString() == "FEAST":
            linear_solver_settings = self.eigensolver_settings["linear_solver_settings"]
            if linear_solver_settings["solver_type"].GetString() == "skyline_lu":
                # default built-in feast system solver
                self.eigen_solver = ExternalSolversApplication.FEASTSolver(self.eigensolver_settings)            
            else:
                # external feast system solver
                feast_system_solver = self._GetFEASTSystemSolver(linear_solver_settings)
                self.eigen_solver = ExternalSolversApplication.FEASTSolver(self.eigensolver_settings, feast_system_solver)
        else:
            raise Exception("solver_type is not yet implemented: " + self.eigensolver_settings["solver_type"].GetString())

        if self.settings["solution_type"].GetString() == "Dynamic":
            self.scheme = StructuralMechanicsApplication.EigensolverDynamicScheme()
        else:
            raise Exception("solution_type is not yet implemented.")

        self.builder_and_solver = KratosMultiphysics.ResidualBasedBlockBuilderAndSolver(self.eigen_solver)

        self.solver = StructuralMechanicsApplication.EigensolverStrategy(
            self.compute_model_part,
            self.scheme,
            self.builder_and_solver)

    def AddVariables(self):
        
        structural_mechanics_solver.MechanicalSolver.AddVariables(self)
   
        print("::[Structural EigenSolver]:: Variables ADDED")

    def AddDofs(self):
        KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.DISPLACEMENT_X, KratosMultiphysics.REACTION_X,self.main_model_part)
        KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.DISPLACEMENT_Y, KratosMultiphysics.REACTION_Y,self.main_model_part)
        KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.DISPLACEMENT_Z, KratosMultiphysics.REACTION_Z,self.main_model_part)
            
        if self.settings["rotation_dofs"].GetBool():
            KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.ROTATION_X, KratosMultiphysics.TORQUE_X,self.main_model_part)
            KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.ROTATION_Y, KratosMultiphysics.TORQUE_Y,self.main_model_part)
            KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.ROTATION_Z, KratosMultiphysics.TORQUE_Z,self.main_model_part)
                
        if self.settings["pressure_dofs"].GetBool():                
            for node in self.main_model_part.Nodes:
                node.AddDof(KratosMultiphysics.PRESSURE, StructuralMechanicsApplication.PRESSURE_REACTION)

        print("::[Structural EigenSolver]:: DOF's ADDED")

    def GetComputeModelPart(self):
        
        return self.main_model_part
    
    def Solve(self):
            
        self.solver.Solve()

    def SetEchoLevel(self, level):

        self.solver.SetEchoLevel(level)

    def _GetFEASTSystemSolver(self, settings):

        if (settings["solver_type"].GetString() == "pastix"):
            return ExternalSolversApplication.PastixComplexSolver(settings)
        else:
            raise Exception("Unsupported feast system solver: " + settings["solver_type"].GetString())
