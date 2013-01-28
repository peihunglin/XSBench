#include "XSbench_header.h"

// Calculates the microscopic cross section for a given nuclide & energy
void calculate_micro_xs( int p_energy, int nuc, int n_isotopes,
                           int n_gridpoints,
                           GridPoint * restrict energy_grid,
                           NuclideGridPoint ** restrict nuclide_grids,
                           int idx, double * restrict xs_vector ){
	
	// pull ptr from energy grid
	NuclideGridPoint * low = energy_grid[idx].xs_ptrs[nuc];
	NuclideGridPoint * high = low + 1;
	
	// assignments
	double e_h, e_l, xs_h, xs_l, e;
	e = energy_grid[idx].energy;
	e_h= high->energy;
	e_l = low->energy;

	// Total XS
	xs_h = high->total_xs;
	xs_l = low->total_xs;
	xs_vector[0] = xs_h - (e_h - e) * (xs_h - xs_l) / (e_h - e_l);
	#ifdef ADD_EXTRAS
	do_flops();
	do_loads( nuc, nuclide_grids, n_gridpoints );	
	#endif
	// Elastic XS
	xs_h = high->elastic_xs;
	xs_l = low->elastic_xs;
	xs_vector[1] = xs_h - (e_h - e) * (xs_h - xs_l) / (e_h - e_l);
	#ifdef ADD_EXTRAS
	do_flops();
	do_loads( (nuc+1) % n_isotopes, nuclide_grids, n_gridpoints );	
	#endif
	// Absorbtion XS
	xs_h = high->absorbtion_xs;
	xs_l = low->absorbtion_xs;
	xs_vector[2] = xs_h - (e_h - e) * (xs_h - xs_l) / (e_h - e_l);
	#ifdef ADD_EXTRAS
	do_flops();
	do_loads( (nuc+2) % n_isotopes, nuclide_grids, n_gridpoints );	
	#endif
	// Fission XS
	xs_h = high->fission_xs;
	xs_l = low->fission_xs;
	xs_vector[3] = xs_h - (e_h - e) * (xs_h - xs_l) / (e_h - e_l);
	#ifdef ADD_EXTRAS
	do_flops();
	do_loads( (nuc+3) % n_isotopes, nuclide_grids, n_gridpoints );	
	#endif
	// Nu Fission XS
	xs_h = high->nu_fission_xs;
	xs_l = low->nu_fission_xs;
	xs_vector[4] = xs_h - (e_h - e) * (xs_h - xs_l) / (e_h - e_l);
	#ifdef ADD_EXTRAS
	do_flops();
	do_loads( (nuc+4) % n_isotopes, nuclide_grids, n_gridpoints );	
	#endif
	
	//test
	/*
	printf("Lookup: Energy = %lf, nuc = %d\n", e, nuc);
	printf("e_h = %lf e_l = %lf\n", e_h, e_l);
	printf("xs_h = %lf xs_l = %lf\n", xs_h, xs_l);
	printf("total_xs = %lf\n", xs_vector[0]);
	*/
}

// Calculates macroscopic cross section based on a given material & energy 
void calculate_macro_xs( double p_energy, int mat, int n_isotopes,
                         int n_gridpoints, int * restrict num_nucs,
                         double ** restrict concs,
                         GridPoint * restrict energy_grid,
                         NuclideGridPoint ** restrict nuclide_grids,
                         int ** restrict mats,
                         double * restrict macro_xs_vector ){
	double xs_vector[5];
	int p_nuc; // the nuclide we are looking up
	int idx = 0;	
	double conc; // the concentration of the nuclide in the material

	// cleans out macro_xs_vector
	for( int k = 0; k < 5; k++ )
		macro_xs_vector[k] = 0;

	// binary search for energy on unionized energy grid (UEG)
	idx = grid_search( n_isotopes * n_gridpoints, p_energy,
	                   energy_grid);	
	
	// Once we find the pointer array on the UEG, we can pull the data
	// from the respective nuclide grids, as well as the nuclide
	// concentration data for the material
	// Each nuclide from the material needs to have its micro-XS array
	// looked up & interpolatied (via calculate_micro_xs). Then, the
	// micro XS is multiplied by the concentration of that nuclide
	// in the material, and added to the total macro XS array.
	for( int j = 0; j < num_nucs[mat]; j++ )
	{
		p_nuc = mats[mat][j];
		conc = concs[mat][j];
		calculate_micro_xs( p_energy, p_nuc, n_isotopes,
		                    n_gridpoints, energy_grid,
		                    nuclide_grids, idx, xs_vector );
		for( int k = 0; k < 5; k++ )
			macro_xs_vector[k] += xs_vector[k] * conc;
	}
	
	//test
	/*
	for( int k = 0; k < 5; k++ )
		printf("Energy: %lf, Material: %d, XSVector[%d]: %lf\n",
		       p_energy, mat, k, macro_xs_vector[k]);
	*/
	
}

// binary search for energy on unionized energy grid
int grid_search( int n, double quarry, GridPoint * A)
{
	int min = 0;
	int max = n-1;
	int mid = 0;
	
	//printf("\nQuarry: %lf\n", quarry);
		
	while( max > min + 1 )
	{
		mid = min + floor( (max-min) / 2.0);
		if( A[mid].energy < quarry )
			min = mid;
		else if( A[mid].energy > quarry )
			max = mid;
		else
			return mid;
	}
	
	//printf("Returning mid: %lf (index %d of %d)\n", A[mid].energy, mid, n);
	return mid;
}
