//
// Created by marechaljas on 07/04/23.
//
#define BOOST_TEST_MODULE rhsTimeSeries
#define BOOST_TEST_DYN_LINK
#define WIN32_LEAN_AND_MEAN

#include <boost/test/unit_test.hpp>
#include <antares/study/study.h>
#include <filesystem>
#include <fstream>
#include "utils.h"

using namespace Antares::Solver;
using namespace Antares::Data;
namespace fs = std::filesystem;

void initializeStudy(Study& study)
{
    study.parameters.derated = false;

    study.runtime = new StudyRuntimeInfos();
    study.runtime->rangeLimits.year[rangeBegin] = 0;
    study.runtime->rangeLimits.year[rangeEnd] = 0;
    study.runtime->rangeLimits.year[rangeCount] = 1;

    study.parameters.renewableGeneration.toAggregated(); // Default

    study.parameters.intraModal = 0;
    study.parameters.interModal = 0;
    study.parameters.timeSeriesToRefresh = 0;
}

class PublicStudy: public Study {
public:
    bool internalLoadBindingConstraints(const StudyLoadOptions& options) override {
        return Study::internalLoadBindingConstraints(options);
    }
};

template<class Ta, class Tb>
void CheckEqual(const Matrix<Ta>& a, const Matrix<Tb>& b) {
    BOOST_CHECK_EQUAL(a.width, b.width);
    BOOST_CHECK_EQUAL(a.height, b.height);
    if (a.height > 0 && a.width > 0) {
        BOOST_CHECK_EQUAL(a[0][0], b[0][0]);
        BOOST_CHECK_EQUAL(a[a.width-1][a.height-1], b[b.width-1][b.height-1]);
    }
}

struct Fixture {
    Fixture() {
        study->header.version = version870;
        working_tmp_dir = CREATE_TMP_DIR_BASED_ON_TEST_NAME();

        study->folderInput = working_tmp_dir.string();
        fs::create_directories(working_tmp_dir / "bindingconstraints");

        addConstraint("dummy_name", "dummy_group");
        initializeStudy(*study);

        expected_lower_bound_series.resize(3, 8784);
        expected_upper_bound_series.resize(3, 8784);
        expected_equality_series.resize(3, 8784);

        expected_lower_bound_series.fillColumn(0, 0.3);
        expected_lower_bound_series.fillColumn(1, 0.5);
        expected_lower_bound_series.fillColumn(2, 1);

        expected_upper_bound_series.fillColumn(0, 0.2);
        expected_upper_bound_series.fillColumn(1, 0.6);
        expected_upper_bound_series.fillColumn(2, 0);

        expected_equality_series.fillColumn(0, 0.1);
        expected_equality_series.fillColumn(1, 0.55);
        expected_equality_series.fillColumn(2, 0.9);
        expected_equality_series[0][8763] = 1;

        expected_lower_bound_series.saveToCSVFile((working_tmp_dir / "bindingconstraints"/ "dummy_name_lt.txt").string());
        expected_upper_bound_series.saveToCSVFile((working_tmp_dir / "bindingconstraints"/ "dummy_name_gt.txt").string());
        expected_equality_series.saveToCSVFile((working_tmp_dir / "bindingconstraints"/ "dummy_name_eq.txt").string());
    };

    void addConstraint(const std::string& name, const std::string& group, bool reset = false) const {
        std::ofstream constraints(working_tmp_dir / "bindingconstraints" / "bindingconstraints.ini",
                                  reset ? std::ios_base::out : std::ios_base::app);
        static unsigned constraintNumber = 1;
        if (reset) constraintNumber = 1;
        constraints << "[" << constraintNumber++ << "]\n"
                    << "name = " << name << "\n"
                    <<"id = " << name << "\n"
                    << "enabled = false\n"
                    << "type = hourly\n"
                    << "operator = equal\n"
                    << "filter-year-by-year = annual\n"
                    << "filter-synthesis = hourly\n"
                    << "comments = dummy_comment\n"
                    << "group = " << group << "\n"
                ;
        constraints.close();
        std::ofstream rhs(working_tmp_dir / "bindingconstraints"/ (name+"_eq.txt"));
        rhs.close();
    }

    std::shared_ptr<PublicStudy> study = std::make_shared<PublicStudy>();
    StudyLoadOptions options;
    std::filesystem::path working_tmp_dir;;
    Matrix<double> expected_lower_bound_series;
    Matrix<double> expected_upper_bound_series;
    Matrix<double> expected_equality_series;
};

BOOST_AUTO_TEST_SUITE(BC_TimeSeries)

BOOST_FIXTURE_TEST_CASE(load_binding_constraints_timeseries, Fixture) {
    bool loading_ok = study->internalLoadBindingConstraints(options);
    BOOST_CHECK_EQUAL(loading_ok, true);
    BOOST_CHECK_EQUAL(study->bindingConstraints.size(), 1);
    CheckEqual(study->bindingConstraints.find("dummy_name")->RHSTimeSeries(), expected_equality_series);

    {
        std::ofstream constraints(working_tmp_dir / "bindingconstraints"/ "bindingconstraints.ini");
        constraints << "[1]\n"
                    << "name = dummy_name\n"
                    << "id = dummy_name\n"
                    << "enabled = false\n"
                    << "type = hourly\n"
                    << "operator = less\n"
                    << "group = dummy_group\n";
        constraints.close();
    }
    loading_ok = study->internalLoadBindingConstraints(options);
    BOOST_CHECK_EQUAL(loading_ok, true);
    CheckEqual(study->bindingConstraints.find("dummy_name")->RHSTimeSeries(), expected_lower_bound_series);

    {
        std::ofstream constraints(working_tmp_dir / "bindingconstraints" / "bindingconstraints.ini");
        constraints << "[1]\n"
                    << "name = dummy_name\n"
                    << "id = dummy_name\n"
                    << "enabled = false\n"
                    << "type = hourly\n"
                    << "operator = greater\n"
                    << "group = dummy_group\n";
        constraints.close();
    }
    loading_ok = study->internalLoadBindingConstraints(options);
    BOOST_CHECK_EQUAL(loading_ok, true);
    CheckEqual(study->bindingConstraints.find("dummy_name")->RHSTimeSeries(), expected_upper_bound_series);
}

BOOST_FIXTURE_TEST_CASE(verify_all_constraints_in_a_group_have_the_same_number_of_time_series_error_case, Fixture) {
    addConstraint("dummy_name_2", "dummy_group");
    Matrix values;
    values.resize(5, 8784);
    values.fill(0.42);
    values.saveToCSVFile((working_tmp_dir / "bindingconstraints"/ "dummy_name_2_eq.txt").string());
    auto loading_ok = study->internalLoadBindingConstraints(options);
    BOOST_CHECK_EQUAL(loading_ok, false);
}

BOOST_FIXTURE_TEST_CASE(verify_all_constraints_in_a_group_have_the_same_number_of_time_series_good_case, Fixture) {
    addConstraint("dummy_name_2", "dummy_group");

    Matrix values;
    values.resize(3, 8784);
    values.fill(0.42);
    values.saveToCSVFile((working_tmp_dir / "bindingconstraints"/ "dummy_name_2_eq.txt").string());
    auto loading_ok = study->internalLoadBindingConstraints(options);
    BOOST_CHECK_EQUAL(loading_ok, true);
}

BOOST_FIXTURE_TEST_CASE(Check_empty_file_interpreted_as_all_zeroes, Fixture) {
    std::vector file_names = {working_tmp_dir / "bindingconstraints"/ "dummy_name_lt.txt",
                              working_tmp_dir / "bindingconstraints"/ "dummy_name_gt.txt",
                              working_tmp_dir / "bindingconstraints"/ "dummy_name_eq.txt"};
    for (auto file_name: file_names) {
        std::ofstream ofs;
        ofs.open(file_name, std::ofstream::out | std::ofstream::trunc);
        ofs.close();
    }

    bool loading_ok = study->internalLoadBindingConstraints(options);
    BOOST_CHECK_EQUAL(loading_ok, true);
    auto expectation = Matrix(1, 8784);
    expectation.fill(0);
    CheckEqual(study->bindingConstraints.find("dummy_name")->RHSTimeSeries(), expectation);
}

BOOST_FIXTURE_TEST_CASE(Check_missing_file, Fixture) {
    std::vector file_names = {working_tmp_dir / "bindingconstraints"/ "dummy_name_lt.txt",
                              working_tmp_dir / "bindingconstraints"/ "dummy_name_gt.txt",
                              working_tmp_dir / "bindingconstraints"/ "dummy_name_eq.txt"};
    for (auto file_name: file_names) {
        std::filesystem::remove(file_name);
    }

    bool loading_ok = study->internalLoadBindingConstraints(options);
    BOOST_CHECK_EQUAL(loading_ok, true);
    BOOST_CHECK_EQUAL(study->bindingConstraints.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
