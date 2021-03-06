/*
 * include/wobbly.h
 *
 * C++ Interface for "wobbly" textures
 *
 * Implicitly depends on:
 *  - std::array
 *  - boost::geometry
 *
 * See LICENCE.md for Copyright information
 */
#ifndef WOBBLY_H
#define WOBBLY_H

#include <cstddef>

#include <array>                        // for array, swap
#include <memory>
#include <stdexcept>                    // for runtime_error
#include <type_traits>
#include <vector>

#include <boost/geometry/geometry.hpp> // IWYU pragma: keep
#include <boost/geometry/core/access.hpp>  // for access
#include <boost/geometry/core/cs.hpp>   // for cartesian
#include <boost/geometry/core/tags.hpp>  // for geometry
#include <boost/geometry/geometries/register/point.hpp>

/* std::swap is explicitly specialized for numerous other types we don't care
 * about */
// IWYU pragma: no_include <__split_buffer>
// IWYU pragma: no_include <__bit_reference>
// IWYU pragma: no_include <__mutex_base>
// IWYU pragma: no_include <__tree>
// IWYU pragma: no_include <deque>
// IWYU pragma: no_include <functional>
// IWYU pragma: no_include <list>
// IWYU pragma: no_include <map>
// IWYU pragma: no_include <set>
// IWYU pragma: no_include <sstream>
// IWYU pragma: no_include <stack>
// IWYU pragma: no_include <string>
// IWYU pragma: no_include <tuple>
// IWYU pragma: no_include <utility>
// IWYU pragma: no_forward_declare wobbly::Anchor::MovableAnchor
// IWYU pragma: no_forward_declare wobbly::Model::Private

namespace wobbly
{
    namespace bg = boost::geometry;

    namespace detail
    {
        struct CopyablePV
        {
            public:

                CopyablePV (CopyablePV const &copy) = default;
                CopyablePV & operator= (CopyablePV const &copy) = default;
        };

        struct NoncopyablePV
        {
            public:

                NoncopyablePV (NoncopyablePV const &copy) = delete;
                NoncopyablePV & operator= (NoncopyablePV const &copy) = delete;
        };

        template <typename NumericType>
        class PointViewCopyabilityBase
        {
            public:
                typedef NumericType NT;
                typedef std::is_const <NumericType> IC;
                typedef std::conditional <IC::value,
                                          CopyablePV,
                                          NoncopyablePV> type;
        };
    }

    template <typename NumericType>
    class PointView :
        public detail::PointViewCopyabilityBase <NumericType>::type
    {
        public:

            typedef NumericType NT;
            typedef typename std::remove_const <NumericType>::type NTM;
            typedef typename std::add_const <NumericType>::type NTC;

            typedef typename std::is_const <NT> IC;

            template <std::size_t N>
            PointView (std::array <NTM, N> &points,
                       std::size_t         index) :
                array (points.data ()),
                offset (index * 2)
            {
            }

            template <std::size_t N,
                      typename = typename std::enable_if <IC::value && N>::type>
            PointView (std::array <NTM, N> const &points,
                       std::size_t               index) :
                array (points.data ()),
                offset (index * 2)
            {
            }

            PointView (NumericType *points,
                       std::size_t index) :
                array (points),
                offset (index * 2)
            {
            }

            PointView (PointView <NumericType> &&view) :
                array (view.array),
                offset (view.offset)
            {
            }

            PointView &
            operator= (PointView <NumericType> &&view) noexcept
            {
                if (this == &view)
                    return *this;

                array = view.array;
                offset = view.offset;

                return *this;
            }

            /* Copy constructor and assignment operator enabled only if the view
             * is truly just observing and has no write access to the point,
             * see detail::PointViewCopyabilityBase */
            PointView (PointView <NumericType> const &p) :
                array (p.array),
                offset (p.offset)
            {
            }

            friend void swap (PointView <NumericType> &first,
                              PointView <NumericType> &second)
            {
                using std::swap;
                swap (first.array, second.array);
                swap (first.offset, second.offset);
            }

            PointView &
            operator= (PointView <NumericType> const &p)
            {
                PointView <NumericType> copy (p);
                swap (*this, copy);
                return *this;
            }

            /* cppcheck thinks this is a constructor, so we need to suppress
             * a constructor warning here.
             */
            // cppcheck-suppress uninitMemberVar
            operator PointView <NTC> ()
            {
                /* The stored offset is an array offset, but the constructor
                 * requires a point offset. Divide by two */
                return PointView <NTC> (array, offset / 2);
            }

            template <size_t Offset>
            NumericType const & get () const
            {
                static_assert (Offset < 2, "Offset must be < 2");
                return array[offset + Offset];
            }

            template <size_t Offset>
            typename std::enable_if <!(IC::value) && (Offset + 1), void>::type
            set (NumericType value)
            {
                static_assert (Offset < 2, "Offset must be < 2");
                array[offset + Offset] = value;
            }

        private:

            NumericType *array;
            size_t      offset;
    };

    struct Point
    {
        Point (double x, double y) noexcept :
            x (x),
            y (y)
        {
        }

        Point () noexcept :
            x (0),
            y (0)
        {
        }

        Point (Point const &p) noexcept :
            x (p.x),
            y (p.y)
        {
        }

        void swap (Point &a, Point &b) noexcept
        {
            using std::swap;

            swap (a.x, b.x);
            swap (a.y, b.y);
        }

        Point & operator= (Point other) noexcept
        {
            swap (*this, other);

            return *this;
        }

        double x;
        double y;
    };

    typedef Point Vector;
}

BOOST_GEOMETRY_REGISTER_POINT_2D (wobbly::Point,
                                  double,
                                  wobbly::bg::cs::cartesian, x, y)

BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET (wobbly::PointView <double>,
                                          double,
                                          wobbly::bg::cs::cartesian,
                                          wobbly::PointView <double>::get <0>,
                                          wobbly::PointView <double>::get <1>,
                                          wobbly::PointView <double>::set <0>,
                                          wobbly::PointView <double>::set <1>);

/* Register the const-version of PointView. We are not using the
 * provided macro as non exists for a const-version */
namespace boost
{
    namespace geometry
    {
        namespace traits
        {
            namespace wobbly
            {
                typedef ::wobbly::PointView <double const> DPV;
            }

            BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS (wobbly::DPV,
                                                           2,
                                                           double,
                                                           cs::cartesian);

            template <>
            struct access <wobbly::DPV, 0>
            {
                static inline double get (wobbly::DPV const &p)
                {
                    return p.get <0> ();
                }
            };

            template <>
            struct access <wobbly::DPV, 1>
            {
                static inline double get (wobbly::DPV const &p)
                {
                    return p.get <1> ();
                }
            };
        }
    }
}

namespace wobbly
{
    class Anchor
    {
        public:

            Anchor ();
            Anchor (Anchor &&) noexcept = default;
            Anchor & operator= (Anchor &&) noexcept = default;
            ~Anchor ();

            void MoveBy (Vector const &delta) noexcept;

            class MovableAnchor;
            struct MovableAnchorDeleter
            {
                void operator () (MovableAnchor *);
            };

            typedef std::unique_ptr <MovableAnchor, MovableAnchorDeleter> Impl;

            static Anchor Create (Impl &&imp);

        protected:

            Anchor (Anchor const &) = delete;
            Anchor & operator= (Anchor const &) = delete;

            Impl priv;
    };

    class Model
    {
        public:

            struct Settings
            {
                double springConstant;
                double friction;
                double maximumRange;
            };

            Model (Point const &initialPosition,
                   double width,
                   double height,
                   Settings const &settings);
            Model (Point const &initialPosition,
                   double width,
                   double height);
            Model (Model const &other);
            ~Model ();

            /* This function will cause a point on the spring mesh closest
             * to grab in absolute terms to become immobile in the mesh.
             *
             * The resulting point can be moved freely by the returned
             * object. Integrating the model after the point has been moved
             * will effectively cause force to be exerted on all the other
             * points. */
            wobbly::Anchor
            GrabAnchor (Point const &grab) throw (std::runtime_error);

            /* This function will insert a new point in the spring
             * mesh which is immobile, with springs from it to its
             * two nearest neighbours.
             *
             * The resulting point can be moved freely by the returned object.
             * As above, integrating the model after moving the point will
             * effectively cause force to be exerted on all other non-immobile
             * points in the mesh */
            wobbly::Anchor
            InsertAnchor (Point const &grab) throw (std::runtime_error);

            /* Performs a single integration per 16 ms in millisecondsDelta */
            bool Step (unsigned int millisecondsDelta);

            /* Takes a normalized texture co-ordinate from 0 to 1 and returns
             * an absolute-position on-screen for that texture co-ordinate
             * as deformed by the model */
            Point DeformTexcoords (Point const &normalized) const;

            /* Bounding box for the model */
            std::array <Point, 4> const Extremes () const;

            /* These functions will attempt to move and resize
             * the model relative to its target position, however,
             * caution should be exercised when using them.
             *
             * A full integration until the model has reached equillibrium
             * may need to be performed in order to determine the target
             * position and given the nature of the calculations, there may
             * be some error in determining that target position. That may
             * affect the result of these operations to a slight degree.
             *
             * Moving and resizing a model will also move any attached anchors
             * relative to the change in the model's mesh. You might want to
             * compensate for this by applying movementto those anchors
             * if they are to stay in the same position.
             *
             * If a precise position is required, then the recommended course
             * of action is to destroy and re-create the model. */
            void MoveModelTo (Point const &point);
            void MoveModelBy (Point const &delta);
            void ResizeModel (double width, double height);

            static constexpr double DefaultSpringConstant = 8.0;
            static constexpr double DefaultObjectRange = 500.0f;
            static constexpr double Mass = 15.0f;
            static constexpr double Friction = 3.0f;

            static Settings DefaultSettings;

        private:

            class Private;
            std::unique_ptr <Private> priv;
    };
}
#endif
