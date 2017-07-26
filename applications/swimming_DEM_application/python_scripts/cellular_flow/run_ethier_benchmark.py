import KratosSwimmingDEM as script
import sys
import ProjectParameters as pp
import DEM_explicit_solver_var as DEM_parameters
varying_parameters = dict()

irregular_mesh_sizes = {0.2}
regular_mesh_n_points = set()
derivatives_types = range(1, 3)
combinations_that_failed = []
for size in irregular_mesh_sizes.union(regular_mesh_n_points):
    varying_parameters['size_parameter'] = size
    for derivatives_type in derivatives_types:
        varying_parameters['material_acceleration_calculation_type'] = derivatives_type
        varying_parameters['laplacian_calculation_type'] = derivatives_type
        import ethier_benchmark_algorithm
        with ethier_benchmark_algorithm.Algorithm(varying_parameters) as algorithm:
            try:
                test = script.Solution(algorithm, varying_parameters)
                test.alg.Run()
            except:
                error = sys.exc_info()
                combinations_that_failed.append({'size':size, 'type':derivatives_type})

print()
print('****************************************')

if len(combinations_that_failed):
    print('The following combinations produced an error:')
    print()
    for combination in combinations_that_failed:
        print(combination)
else:
    print('All combinations run without errors')
print('****************************************')
