///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                           *** Generator.cxx ***                           //
//                                                                           //
// created December 12, 2019                                                 //
// copyright Christopher N. Singh Binghamton University Physics              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <gendat/Generator.hpp>

template <typename T> 
Generator<T>::Generator(std::string model)
	: model(model)
{
	if (model != "ising")
	{
		std::cerr << "CURRENTLY UNSUPPORTED MODEL" << std::endl;
		exit(-1);
	}
}

template <typename T>
void Generator<T>::run(void) const
{
	if (boost::filesystem::exists(fpath)) boost::filesystem::remove(fpath);

	Eigen::Array<T, Eigen::Dynamic, 1> Bx, Bz;

	Bx = Eigen::Array<T, Eigen::Dynamic, 1>::LinSpaced(dBx, Bx_min, Bx_max);
	Bz = Eigen::Array<T, Eigen::Dynamic, 1>::LinSpaced(dBz, Bz_min, Bz_max);

	if (dBz == 1) Bz(0) = 0.01;
	if (dBx == 1) Bx(0) = 0.01;

	T W = replicas > 1 ? disorder : 0;

	Fields<T> fields;
	Solver<T> solver;
	Operators<T> operators(qubits);
	std::chrono::time_point<std::chrono::high_resolution_clock> start, stop, origin;
	std::chrono::duration<double> elapsed;
	origin = std::chrono::high_resolution_clock::now();
	elapsed = origin - origin;

	for (int iBx = 0; iBx < Bx.size(); ++iBx)
	{
		for (int iBz = 0; iBz < Bz.size(); ++iBz)
		{
			for (int iW = 0; iW < replicas; ++iW)
			{
				fields = Fields<T>(qubits, coupling, Bx[iBx], Bz[iBz], W);

				start = std::chrono::high_resolution_clock::now();
				solver.compute(operators.ising_hamiltonian(fields));
				stop = std::chrono::high_resolution_clock::now();
				elapsed += stop - start;

				if (solver.info() != Eigen::Success)
				{
					std::cerr << " \nDIAGONALIZATION FAILED\n\n";
					exit(-1);
				}

				Instance<T>(fields, solver).append_to_file(fpath);
			}
		}
	}
	std::cout << "Diagonalization time: " << elapsed.count() << " seconds\n";
}

template <typename T>
void Generator<T>::prompt_if_file(std::string fpath) const 
{
	if (boost::filesystem::exists(fpath))
	{
		std::cout << " WARNING: FILE EXISTS... REMOVE " 
				  << fpath << " [y/n] \n";

		char choice;
		std::cin >> choice;

		if (choice == 'y')
		{
			std::cout << " OK, REMOVING " << fpath << '\n';
			boost::filesystem::remove(fpath);
		}
		else if (choice == 'n') 
		{
			std::cout << " QUIT OR APPEND ANYWAY? [q/a]\n";
			std::cin >> choice;

			if (choice == 'q')
			{
				std::cout << " OK, QUITTING\n";
				exit(-1);
			}
			else if (choice == 'a')
			{
				std::cout << "CONTINUING\n";
			}
		}
	}
}

template class Generator<double>;
