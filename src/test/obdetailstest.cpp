#include <gtest/gtest.h>
#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include "obdetails.h"

#include <openbabel/math/vector3.h>
#include <openbabel/obconversion.h>
#include <openbabel/obiter.h>
#include <openbabel/atom.h>
#include <openbabel/mol.h>

namespace OBDetailsTest {
    constexpr int numElements{118};
    constexpr int numNonMetals{23};
    constexpr std::array<int, numNonMetals> nonMetalAtomicNums{1, 2, 5, 6, 7, 8, 9, 10, 14, 15, 16, 17, 18, 32, 33, 34, 35, 36, 52, 53, 54, 85, 86};
    const std::map<int, const char*> elements{{1, "Hydro"}, {2, "Heliu"}, {3, "Lithi"}, {8, "Oxyge"}, {14, "Silic"}, {29, "Coppe"}, {39, "Yttri"}, {48, "Cadmi"}, {54, "Xenon"}, {81, "Thall"}, {101, "Mende"}};
}

using namespace OBDetailsTest;

TEST(IsMetalTest, HandlesMetal) {
    int idx{0};
    OpenBabel::OBAtom metalAtom{};
    for (std::size_t atomicNum{1}; atomicNum <= numElements; ++atomicNum) {
        if ((idx < numNonMetals) && (atomicNum == nonMetalAtomicNums[idx]))
            ++idx;
        else {
            metalAtom.SetAtomicNum(atomicNum);
            EXPECT_TRUE(OpenBabel::isMetal(&metalAtom));
        }
    }
}

TEST(IsMetalTest, HandlesNonMetal) {
    OpenBabel::OBAtom nonMetalAtom{};
    for (int atomicNum : nonMetalAtomicNums) {
        nonMetalAtom.SetAtomicNum(atomicNum);
        EXPECT_FALSE(OpenBabel::isMetal(&nonMetalAtom));
    }
}

TEST(FormBondTest, HandlesNullAtom) {
    OpenBabel::OBMol mol{};
    OpenBabel::OBAtom atom{};
    constexpr int bondOrder{1};
    EXPECT_EQ(nullptr, OpenBabel::formBond(&mol, nullptr, &atom, bondOrder));
    EXPECT_EQ(nullptr, OpenBabel::formBond(&mol, &atom, nullptr, bondOrder));
}

TEST(FormBondTest, HandlesBondExists) {
    std::string N2SMILES{"N#N"};
    std::stringstream ss{N2SMILES};
    OpenBabel::OBConversion conv{&ss};
    conv.SetInFormat("SMI");
    OpenBabel::OBMol mol{};
    conv.Read(&mol);
    OpenBabel::OBBond* bond{*(mol.BeginBonds())};
    OpenBabel::OBAtom* nitrogen1{bond->GetBeginAtom()};
    OpenBabel::OBAtom* nitrogen2{bond->GetEndAtom()};
    constexpr int bondOrder{3};
    EXPECT_EQ(nullptr, OpenBabel::formBond(&mol, nitrogen1, nitrogen2, bondOrder)); 
    EXPECT_EQ(nullptr, OpenBabel::formBond(&mol, nitrogen2, nitrogen1, bondOrder)); 
}

TEST(FormBondTest, HandlesCreateBond) {
    OpenBabel::OBMol mol{};
    OpenBabel::OBAtom* atom1{mol.NewAtom()};
    OpenBabel::OBAtom* atom2{mol.NewAtom()};
    constexpr int bondOrder{1};
    OpenBabel::OBBond* bond{OpenBabel::formBond(&mol, atom1, atom2, bondOrder)};
    EXPECT_EQ(bondOrder, bond->GetBondOrder());
    EXPECT_EQ(atom1, bond->GetBeginAtom());
    EXPECT_EQ(atom2, bond->GetEndAtom());
    EXPECT_EQ(&mol, bond->GetParent());
    EXPECT_EQ(bond, atom1->GetBond(atom2));
    EXPECT_EQ(bond, atom2->GetBond(atom1));
}

TEST(FormAtomTest, HandlesAtom) {
    OpenBabel::OBMol mol{};
    const OpenBabel::vector3 loc{3.14159, 2.65358, 9.79323};
    constexpr int atomicNum{8};
    OpenBabel::OBAtom* atom{OpenBabel::formAtom(&mol, loc, atomicNum)};
    EXPECT_EQ(atomicNum, atom->GetAtomicNum());
    EXPECT_EQ(&mol, atom->GetParent());
    EXPECT_EQ(loc, atom->GetVector());
    EXPECT_EQ(atom, mol.GetFirstAtom());
}

TEST(ChangeAtomElementTest, HandlesAtom) {
    OpenBabel::OBAtom atom{};
    for (const auto& p : OBDetailsTest::elements) {
        const int atomicNum{p.first};
        const char* type{p.second};
        OpenBabel::changeAtomElement(&atom, atomicNum);
        EXPECT_EQ(atomicNum, atom.GetAtomicNum());
        EXPECT_STREQ(type, atom.GetType());
    }
}

TEST(DeleteBondsTest, HandlesNonMetal) {
    std::string N2SMILES{"N#N"};
    std::stringstream ss{N2SMILES};
    OpenBabel::OBConversion conv{&ss};
    conv.SetInFormat("SMI");
    OpenBabel::OBMol mol{};
    conv.Read(&mol);
    constexpr bool onlyMetals{false}; 
    EXPECT_EQ(1, OpenBabel::deleteBonds(&mol, onlyMetals));
    EXPECT_EQ(0, mol.NumBonds());
    EXPECT_EQ(2, mol.NumAtoms());
    FOR_ATOMS_OF_MOL(a, mol) {
        EXPECT_EQ(a->BeginBonds(), a->EndBonds());
    }
}

TEST(DeleteBondsTest, HandlesMetalOnly) {
    std::string N2SMILES{"N#N"};
    std::stringstream ss{N2SMILES};
    OpenBabel::OBConversion conv{&ss};
    conv.SetInFormat("SMI");
    OpenBabel::OBMol mol{};
    conv.Read(&mol);
    constexpr bool onlyMetals{true}; 
    EXPECT_EQ(0, OpenBabel::deleteBonds(&mol, onlyMetals));
    EXPECT_EQ(1, mol.NumBonds());
    EXPECT_EQ(2, mol.NumAtoms());
    FOR_ATOMS_OF_MOL(a, mol) {
        EXPECT_NE(a->BeginBonds(), a->EndBonds());
    }
}

TEST(DeleteBondsTest, HandlesMetal) {
    std::string ClCuSMILES{"Cl[Cu]"};
    std::stringstream ss{ClCuSMILES};
    OpenBabel::OBConversion conv{&ss};
    conv.SetInFormat("SMI");
    OpenBabel::OBMol mol{};
    conv.Read(&mol);
    constexpr bool onlyMetals{true};
    EXPECT_EQ(1, OpenBabel::deleteBonds(&mol, onlyMetals));
    EXPECT_EQ(0, mol.NumBonds());
    EXPECT_EQ(2, mol.NumAtoms());
    FOR_ATOMS_OF_MOL(a, mol) {
        EXPECT_EQ(a->BeginBonds(), a->EndBonds());
    }
}

TEST(AtomsEqualTest, HandlesUnequalNum) {
    OpenBabel::OBAtom titanium{};
    titanium.SetAtomicNum(22);
    OpenBabel::OBAtom vanadium{};
    vanadium.SetAtomicNum(23);
    EXPECT_FALSE(OpenBabel::atomsEqual(titanium, vanadium));
}

TEST(AtomsEqualTest, HandlesEqualNum) {
    OpenBabel::OBAtom germanium{};
    germanium.SetAtomicNum(32);
    OpenBabel::OBAtom germanium2{};
    germanium2.SetAtomicNum(32);
    EXPECT_TRUE(OpenBabel::atomsEqual(germanium, germanium2));
}

TEST(AtomsEqualTest, HandlesUnequalVector) {
    OpenBabel::OBAtom atom1{};
    atom1.SetVector(0.12345, 0.67891, 0.31415);
    OpenBabel::OBAtom atom2{};
    atom2.SetVector(0.12345, 0.678912, 0.31415);
    EXPECT_FALSE(OpenBabel::atomsEqual(atom1, atom2));
}

TEST(AtomsEqualTest, HandlesEqualVector) {
    OpenBabel::OBAtom atom1{};
    atom1.SetVector(0.12345, 0.67891, 0.31415);
    OpenBabel::OBAtom atom2{};
    atom2.SetVector(0.12345, 0.67891, 0.31415);
    EXPECT_TRUE(OpenBabel::atomsEqual(atom1, atom2));
    atom1.SetVector(0.12345, 0.6789103, 0.31415);
    atom2.SetVector(0.12345, 0.6789109, 0.31415);
    EXPECT_TRUE(OpenBabel::atomsEqual(atom1, atom2));
}

TEST(AtomsEqualTest, HandlesUnequalIsotope) {
    OpenBabel::OBAtom atom1{};
    atom1.SetIsotope(0);
    OpenBabel::OBAtom atom2{};
    atom2.SetIsotope(1);
    EXPECT_FALSE(OpenBabel::atomsEqual(atom1, atom2));
}

TEST(AtomsEqualTest, HandlesEqualIsotope) {
    OpenBabel::OBAtom atom1{};
    atom1.SetIsotope(0);
    OpenBabel::OBAtom atom2{};
    atom2.SetIsotope(0);
    EXPECT_TRUE(OpenBabel::atomsEqual(atom1, atom2));
}

TEST(AtomInOtherMolTest, HandlesIn) {
    std::string MnO4SMILES{"[O-][Mn](=O)(=O)=O"};
    std::stringstream ss{MnO4SMILES};
    OpenBabel::OBConversion conv{&ss};
    conv.SetInFormat("SMI");
    OpenBabel::OBMol mol{};
    conv.Read(&mol);
    OpenBabel::OBAtom oxygen{};
    oxygen.SetAtomicNum(8);
    OpenBabel::OBAtom* copycatOxygen{OpenBabel::atomInOtherMol(&oxygen, &mol)};
    EXPECT_EQ(copycatOxygen->GetAtomicNum(), oxygen.GetAtomicNum());
    OpenBabel::OBAtom manganese{};
    manganese.SetAtomicNum(25);
    OpenBabel::OBAtom* copycatManganese{OpenBabel::atomInOtherMol(&manganese, &mol)};
    EXPECT_EQ(copycatManganese->GetAtomicNum(), manganese.GetAtomicNum());
}

TEST(AtomInOtherMolTest, HandlesNotIn) {
    std::string MICSMILES{"CN=C=O"};
    std::stringstream ss{MICSMILES};
    OpenBabel::OBConversion conv{&ss};
    conv.SetInFormat("SMI");
    OpenBabel::OBMol mol{};
    conv.Read(&mol);
    OpenBabel::OBAtom fluorine{};
    fluorine.SetAtomicNum(9);
    EXPECT_EQ(nullptr, OpenBabel::atomInOtherMol(&fluorine, &mol));
    OpenBabel::OBAtom boron{};
    boron.SetAtomicNum(5);
    EXPECT_EQ(nullptr, OpenBabel::atomInOtherMol(&boron, &mol));
}

TEST(RTrimWhiteSpaceTest, HandlesEmptyString) {
    EXPECT_EQ("", OpenBabel::rtrimWhiteSpace(""));
}

TEST(RTrimWhiteSpaceTest, HandlesTrimmedString) {
    EXPECT_EQ("Hi", OpenBabel::rtrimWhiteSpace("Hi"));
    EXPECT_EQ("Bye", OpenBabel::rtrimWhiteSpace("Bye"));
}

TEST(RTrimWhiteSpaceTest, HandlesUntrimmedString) {
    EXPECT_EQ("Hi", OpenBabel::rtrimWhiteSpace("Hi       "));
    EXPECT_EQ("Bye", OpenBabel::rtrimWhiteSpace("Bye "));
    EXPECT_EQ("     Hi", OpenBabel::rtrimWhiteSpace("     Hi"));
    EXPECT_EQ(" Bye", OpenBabel::rtrimWhiteSpace(" Bye"));
}
