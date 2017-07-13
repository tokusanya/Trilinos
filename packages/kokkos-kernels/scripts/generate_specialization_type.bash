#!/bin/bash
Function=$1                 #e.g. abs: function name
FunctionExtended=$2         #e.g. KokkosBlas1_impl_MV_abs: prefix for files etc.
Scalar=$3                   #e.g. double
Layout=$4                   #e.g. LayoutLeft
ExecSpace=$5                #e.g. OpenMP
MemSpace=$6                 #e.g. HostSpace
filename_master_hpp=$7      #e.g. Kokkos_Blas1_MV_impl_abs.hpp: where the actual function definition and declaration lives 
NameSpace=$8                #e.g. KokkosBlas: namespace it lives in
KokkosKernelsPath=$9        

Macro=`echo ${FunctionExtended} | awk '{print toupper($0)}'`
Scalar_UpperCase=`echo ${Scalar} | awk '{print toupper($0)}' | sed 's|\:\:|\_|g' | sed 's|<|_|g' | sed 's|>|_|g'`
Scalar_FileName=`echo ${Scalar} | sed 's|\:\:|\_|g' | sed 's|<|_|g' | sed 's|>|_|g'`
Layout_UpperCase=`echo ${Layout} | awk '{print toupper($0)}'`
ExecSpace_UpperCase=`echo ${ExecSpace} | awk '{print toupper($0)}'`
MemSpace_UpperCase=`echo ${MemSpace} | awk '{print toupper($0)}'`

filename_cpp=generated_specializations_cpp/${Function}/${FunctionExtended}_inst_specialization_${Scalar_FileName}_${Layout}_${ExecSpace}_${MemSpace}.cpp
filename_hpp=generated_specializations_hpp/${FunctionExtended}_decl_specializations.hpp


cat ${KokkosKernelsPath}/scripts/header > ${filename_cpp}
echo "" >> ${filename_cpp}
echo "#include \"${filename_master_hpp}\"" >> ${filename_cpp}
echo "" >> ${filename_cpp}
echo "// Turning off the KOKKOSKERNELS_ETI_ONLY macro" >> ${filename_cpp}
echo "// since the core cpp files have to be build always." >> ${filename_cpp}
echo "#ifdef KOKKOSKERNELS_ETI_ONLY" >> ${filename_cpp}
echo "#undef KOKKOSKERNELS_ETI_ONLY" >> ${filename_cpp}
echo "#endif" >> ${filename_cpp}
echo "" >> ${filename_cpp}
echo "namespace ${NameSpace} {" >> ${filename_cpp}
echo "namespace Impl {" >> ${filename_cpp}
echo "#if defined (KOKKOSKERNELS_INST_${Scalar_UpperCase}) \\" >> ${filename_cpp} 
echo " && defined (KOKKOSKERNELS_INST_${Layout_UpperCase}) \\" >> ${filename_cpp} 
echo " && defined (KOKKOSKERNELS_INST_EXECSPACE_${ExecSpace_UpperCase}) \\" >> ${filename_cpp} 
echo " && defined (KOKKOSKERNELS_INST_MEMSPACE_${MemSpace_UpperCase})" >> ${filename_cpp} 
echo " ${Macro}_DEF(${Scalar}, Kokkos::${Layout}, Kokkos::${ExecSpace}, Kokkos::${MemSpace})" >> ${filename_cpp}
echo "#endif" >> ${filename_cpp}
echo "} // Impl" >> ${filename_cpp} 
echo "} // ${NameSpace}" >> ${filename_cpp}

echo "" >> ${filename_hpp}
echo "#if defined (KOKKOSKERNELS_INST_${Scalar_UpperCase}) \\" >> ${filename_hpp}
echo " && defined (KOKKOSKERNELS_INST_${Layout_UpperCase}) \\" >> ${filename_hpp}
echo " && defined (KOKKOSKERNELS_INST_EXECSPACE_${ExecSpace_UpperCase}) \\" >> ${filename_hpp}
echo " && defined (KOKKOSKERNELS_INST_MEMSPACE_${MemSpace_UpperCase})" >> ${filename_hpp}
echo " ${Macro}_DECL(${Scalar}, Kokkos::${Layout}, Kokkos::${ExecSpace}, Kokkos::${MemSpace})" >> ${filename_hpp}
echo "#endif" >> ${filename_hpp}
