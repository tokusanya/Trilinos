#include "Teuchos_UnitTestHarness.hpp"
#include "MueLu_TestHelpers.hpp"
#include "MueLu_Version.hpp"

#include "MueLu_SaPFactory.hpp"

#include "MueLu_UseDefaultTypes.hpp"
#include "MueLu_UseShortNames.hpp"

#include "MueLu_ExtendedHashtable.hpp"

namespace MueLuTests {

  TEUCHOS_UNIT_TEST(ExtendedHashtable, ptrTests)
  {
    out << "version: " << MueLu::Version() << std::endl;

    RCP<SaPFactory> sapFactory = rcp(new SaPFactory);
    TEUCHOS_TEST_EQUALITY(sapFactory != Teuchos::null, true, out, success);

    RCP<SaPFactory> sapFactory2 = rcp(new SaPFactory);
    TEUCHOS_TEST_EQUALITY(sapFactory2 != Teuchos::null, true, out, success);

    RCP<const Teuchos::Comm<int> > comm = Teuchos::DefaultComm<int>::getComm();

    // build Operator
    Teuchos::ParameterList params;
    const RCP<const Map> map = MapFactory::Build(Xpetra::UseTpetra, 20, 0, comm);
    RCP<Operator> Op = MueLu::Gallery::CreateCrsMatrix<SC, LO, GO, Map, CrsOperator>("Laplace1D", map, params);

    // an extended hashtable
    RCP<MueLu::UTILS::ExtendedHashtable> exh = Teuchos::rcp(new MueLu::UTILS::ExtendedHashtable());


    exh->Set<RCP<Operator> >("op",Op,sapFactory.get());
    RCP<Operator> test = exh->Get<RCP<Operator> > ("op",sapFactory.get());
    TEUCHOS_TEST_EQUALITY_CONST( test, Op, out, success );

    exh->Set("op",22,sapFactory.get());
    int test2 = exh->Get<int> ("op",sapFactory.get());
    TEUCHOS_TEST_EQUALITY_CONST( test2, 22, out, success );

    exh->Set("op",Op,sapFactory2.get());
    RCP<Operator> test3 = exh->Get<RCP<Operator> > ("op",sapFactory2.get());
    TEUCHOS_TEST_EQUALITY_CONST( test3, Op, out, success );
    TEUCHOS_TEST_EQUALITY_CONST( exh->GetType("op", sapFactory.get()), "int", out, success );

    exh->Remove("op",sapFactory2.get());
    TEUCHOS_TEST_EQUALITY_CONST( exh->isKey("op",sapFactory.get()),  true, out, success );
    TEUCHOS_TEST_EQUALITY_CONST( exh->isKey("op",sapFactory2.get()), false, out, success );

    exh->Remove("op",sapFactory.get());
    TEUCHOS_TEST_EQUALITY_CONST( exh->isKey("op",sapFactory.get()),  false, out, success );
    TEUCHOS_TEST_EQUALITY_CONST( exh->isKey("op",sapFactory2.get()), false, out, success );


  }

}//namespace MueLuTests

