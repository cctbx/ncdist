######################################################################
#  Makefile - makefile to create sfdist                              #
#                                                                    #
# Version 1.0.0 07 January 2016                                      #
#                                                                    #
#         Herbert J. Bernstein (yayahjb@gmail.com)                   #
#         Lawrence C Andrews                                         #
#                                                                    #
# (C) Copyright 2016 - 2017 Herbert J. Bernstein, Lawrence C. Andrews#
#                                                                    #
######################################################################

######################################################################
#                                                                    #
# YOU MAY REDISTRIBUTE THE sfdist PACKAGE UNDER THE TERMS OF THE GPL #
#                                                                    #
# ALTERNATIVELY YOU MAY REDISTRIBUTE THE sfdist API UNDER THE TERMS  #
# OF THE LGPL                                                        #
#                                                                    #
######################################################################

########################### GPL NOTICES ##############################
#                                                                    #
# This program is free software; you can redistribute it and/or      #
# modify it under the terms of the GNU General Public License as     #
# published by the Free Software Foundation; either version 2 of     #
# (the License, or (at your option) any later version.               #
#                                                                    #
# This program is distributed in the hope that it will be useful,    #
# but WITHOUT ANY WARRANTY; without even the implied warranty of     #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      #
# GNU General Public License for more details.                       #
#                                                                    #
# You should have received a copy of the GNU General Public License  #
# along with this program; if not, write to the Free Software        #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA           #
# 02111-1307  USA                                                    #
#                                                                    #
######################################################################

######################### LGPL NOTICES ###############################
#                                                                    #
# This library is free software; you can redistribute it and/or      #
# modify it under the terms of the GNU Lesser General Public         #
# License as published by the Free Software Foundation; either       #
# version 2.1 of the License, or (at your option) any later version. #
#                                                                    #
# This library is distributed in the hope that it will be useful,    #
# but WITHOUT ANY WARRANTY; without even the implied warranty of     #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  #
# Lesser General Public License for more details.                    #
#                                                                    #
# You should have received a copy of the GNU Lesser General Public   #
# License along with this library; if not, write to the Free         #
# Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,    #
# MA  02110-1301  USA                                                #
#                                                                    #
######################################################################

ifeq ($(OS),Windows_NT)
    uname_S := Windows
else
    uname_S := $(shell uname -s)
endif

ifeq ($(uname_S), Darwin)
  RCPP_HEADERS ?= /Library/Frameworks/R.framework/Versions/3.3/Resources/library/Rcpp/include
  RCPPARMA_HEADERS ?=  /Library/Frameworks/R.framework/Versions/3.3/Resources/library/RcppArmadillo/include
  RCPPPARA_HEADERS ?= /Library/Frameworks/R.framework/Versions/3.3/Resources/library/RcppParallel/include
  RPATH_HEADERS ?= /Library/Frameworks/R.framework/Versions/3.3/Headers
  RPATH_LIBRARIES ?= /Library/Frameworks/R.framework/Versions/3.3/Resources/lib
else
  RCPP_HEADERS ?= /usr/lib/R/site-library/Rcpp/include
  RCPPARMA_HEADERS ?= /usr/lib/R/site-library/RcppArmadillo/include
  RCPPPARA_HEADERS ?= /usr/local/lib/R/site-library/RcppParallel/include
  RPATH_HEADERS ?= /usr/share/R/include
  RPATH_LIBRARIES ?= /usr/lib/R/lib
endif

CFLAGS ?= -g -O3 -fopenmp
CXXFLAGS ?= -g -O3 -fopenmp

all:  ncdist D7Test Follower rcpp_ncdist.so

ncdist_.o:  ncdist.cpp Reducer.h Delone.h Cell.h NCDist.h VecN.h Vec_N_Tools.h
	g++ $(CXXFLAGS) -c -I $(RCPPARMA_HEADERS) -I $(RCPPPARA_HEADERS) ncdist.cpp -o ncdist_.o

ncdist:  ncdist_.o \
	Cell.cpp D6.cpp D7.cpp Delone.cpp DeloneTetrahedron.cpp G6.cpp \
	MatN.cpp MatMN.cpp Mat66.cpp Reducer.cpp \
	VecN.cpp Vec_N_Tools.cpp VectorTools.cpp \
	inverse.cpp vector_3d.cpp \
	Cell.h D6.h D7.h Delone.h DeloneTetrahedron.h G6.h \
        MatN.h MatMN.h Mat66.h Reducer.h \
	VecN.h Vec_N_Tools.h VectorTools.h inverse.cpp vector_3d.cpp 
	g++ $(CXXFLAGS) -o ncdist ncdist_.o -I $(RCPPARMA_HEADERS) -I $(RCPPPARA_HEADERS) \
	Reducer.cpp D6.cpp D7.cpp Delone.cpp DeloneTetrahedron.cpp Cell.cpp G6.cpp \
	MatN.cpp MatMN.cpp Mat66.cpp VecN.cpp Vec_N_Tools.cpp VectorTools.cpp \
	inverse.cpp vector_3d.cpp  -lpthread

rcpp_ncdist_.o:  rcpp_ncdist.cpp Reducer.h Cell.h NCDist.h
	g++ $(CXXFLAGS)  -I$(RPATH_HEADERS) -DNDEBUG  -fpic  -O2 -fPIC \
	 -I$(RCPP_HEADERS) \
		-I$(RCPPPARA_HEADERS) \
		-I$(RCPPARMA_HEADERS) -c rcpp_ncdist.cpp -o rcpp_ncdist_.o

Reducer.o:  Reducer.cpp Reducer.h Cell.h NCDist.h
	g++ $(CXXFLAGS)  -I$(RPATH_HEADERS) -DNDEBUG  -fpic  -O2 -fPIC \
	 -I$(RCPP_HEADERS) \
		-I$(RCPPPARA_HEADERS) \
		-I$(RCPPARMA_HEADERS) -c Reducer.cpp -o Reducer.o


Delone.o:  Delone.cpp Delone.h DeloneTetrahedron.h Reducer.h Cell.h NCDist.h
	g++ $(CXXFLAGS)  -I$(RPATH_HEADERS) -DNDEBUG  -fpic  -O2 -fPIC \
	 -I$(RCPP_HEADERS) \
		-I$(RCPPPARA_HEADERS) \
		-I$(RCPPARMA_HEADERS) -c Delone.cpp DeloneTetrahedron.cpp -o Delone.o

Cell.o:  Cell.cpp Reducer.h Cell.h NCDist.h
	g++ $(CXXFLAGS)  -I$(RPATH_HEADERS) -DNDEBUG  -fpic  -O2 -fPIC \
	 -I$(RCPP_HEADERS) \
		-I$(RCPPPARA_HEADERS) \
		-I$(RCPPARMA_HEADERS) -c Cell.cpp -o Cell.o

rcpp_ncdist.so:	rcpp_ncdist_.o Reducer.o Cell.o
	g++ $(CXXFLAGS) -shared -o rcpp_ncdist.so rcpp_ncdist_.o -I $(RCPPPARA_HEADERS) \
        Reducer.cpp D6.cpp D7.cpp Delone.cpp DeloneTetrahedron.cpp Cell.cpp G6.cpp \
        MatN.cpp MatMN.cpp Mat66.cpp VecN.cpp Vec_N_Tools.cpp VectorTools.cpp \
        inverse.cpp vector_3d.cpp -L$(RPATH_LIBRARIES) -lR -lblas -llapack -lpthread

NCDist.o:  NCDist.c NCDist.h
	gcc $(CFLAGS) -c NCDist.c

D7Dist.o:  D7Dist.c D7Dist.h
	gcc $(CXXFLAGS) -c D7Dist.c

D7Test:  Cell.cpp CellInputData.cpp D6.cpp D7.cpp D7Test.cpp Delone.cpp DeloneTetrahedron.cpp \
	G6.cpp LatticeConverter.cpp Mat66.cpp Mat7.cpp MatMN.cpp MatN.cpp MaximaTools.cpp PrintTable.cpp \
	ProjectorTools.cpp RandTools.cpp ReadCellData.cpp Reducer.cpp StringTools.cpp VecN.cpp \
	Vec_N_Tools.cpp VectorTools.cpp inverse.cpp vector_3d.cpp D7Dist.o NCDist.o \
	Cell.h CellInputData.h CreateFileName.h D6.h D7.h \
	D7Dist.h D7Dist_.hpp Delone.h DeloneTetrahedron.h \
	G6.h Glitch.h GlitchElement.h LinearAxis.h \
	MapBoundaryStrings2Colors.h Mat66.h Mat7.h \
	MatMN.h MatN.h MaximaTools.h NCDist.h NCDist_.hpp \
	NormedInt.h ProjectorTools.h RandTools.h ReadCellData.h \
	Reducer.h ReportTools.h Rn_Path.h \
	StoreResults.h SuperFastHash.h TNear.h Theta.h \
	ToString.h VecN.h Vec_N_Tools.h VectorTools.h \
	cqrlib.h inverse.h rhrand.h triple.h vector_3d.h
	g++ $(CXXFLAGS) -o D7Test  D7Dist.o NCDist.o \
	Cell.cpp CellInputData.cpp D6.cpp D7.cpp D7Test.cpp Delone.cpp DeloneTetrahedron.cpp \
	G6.cpp LatticeConverter.cpp Mat66.cpp Mat7.cpp MatMN.cpp MatN.cpp MaximaTools.cpp PrintTable.cpp \
	ProjectorTools.cpp RandTools.cpp ReadCellData.cpp Reducer.cpp StringTools.cpp VecN.cpp \
	Vec_N_Tools.cpp VectorTools.cpp inverse.cpp vector_3d.cpp -lpthread

Follower: Cell.cpp CellInputData.cpp CreateFileName.cpp D6.cpp D7.cpp \
	Delone.cpp DeloneTetrahedron.cpp FileOperations.cpp \
	FollowerConstants.cpp FollowerIO.cpp FollowerTools.cpp \
	Follower_main.cpp G6.cpp LinearAxis.cpp MapBoundaryStrings2Colors.cpp \
	Mat66.cpp Mat7.cpp MatMN.cpp MatN.cpp MaximaTools.cpp \
	ProjectorTools.cpp RandTools.cpp ReadCellData.cpp \
	ReadGlobalData.cpp Reducer.cpp ReportTools.cpp \
	SuperFastHash.cpp VecN.cpp Vec_N_Tools.cpp VectorTools.cpp \
	inverse.cpp vector_3d.cpp \
	D7Dist.o NCDist.o \
	Cell.h CellInputData.h CreateFileName.h D6.h D7.h \
	D7Dist.h D7Dist_.hpp Delone.h DeloneTetrahedron.h \
	FileOperations.h FileWriter.h Follow.h Follower.h \
	FollowerConstants.h FollowerIO.h FollowerTools.h \
	G6.h Glitch.h GlitchElement.h LinearAxis.h \
	MapBoundaryStrings2Colors.h Mat66.h Mat7.h \
	MatMN.h MatN.h MaximaTools.h NCDist.h NCDist_.hpp \
	NormedInt.h ProjectorTools.h RandTools.h ReadCellData.h \
	ReadGlobalData.h Reducer.h ReportTools.h Rn_Path.h \
	SVG_CirclePlot.h SVG_DistancePlot.h SVG_Writer.h \
	StoreResults.h SuperFastHash.h TNear.h Theta.h \
	ToString.h VecN.h Vec_N_Tools.h VectorTools.h \
	cqrlib.h inverse.h rhrand.h triple.h vector_3d.h
	g++ $(CXXFLAGS) -o Follower \
	Cell.cpp CellInputData.cpp CreateFileName.cpp D6.cpp D7.cpp \
	Delone.cpp DeloneTetrahedron.cpp FileOperations.cpp \
	FollowerConstants.cpp FollowerIO.cpp FollowerTools.cpp \
	Follower_main.cpp G6.cpp LinearAxis.cpp MapBoundaryStrings2Colors.cpp \
	Mat66.cpp Mat7.cpp MatMN.cpp MatN.cpp MaximaTools.cpp \
	ProjectorTools.cpp RandTools.cpp ReadCellData.cpp \
	ReadGlobalData.cpp Reducer.cpp ReportTools.cpp \
	SuperFastHash.cpp VecN.cpp Vec_N_Tools.cpp VectorTools.cpp \
	inverse.cpp vector_3d.cpp \
	D7Dist.o NCDist.o 

clean:
	-@rm -rf *.o
	-@rm -rf *.so
	-@rm -rf D7Test
	-@rm -rf Follower

distclean:  clean
	-@rm -rf bin
	-@rm -rf lib
	-@rm -rf build



