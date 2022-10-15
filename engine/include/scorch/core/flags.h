#pragma once

namespace SC
{
	template <typename EnumT>
	class Flags {
		static_assert(std::is_enum_v<EnumT>, "Flags can only be specialized for enum types");
		using UnderlyingT = typename std::make_unsigned_t<typename std::underlying_type_t<EnumT>>;
	public:
		Flags& set(EnumT e, bool value = true) noexcept {
			m_bits.set(underlying(e), value);
			return *this;
		}

		Flags& reset(EnumT e) noexcept {
			set(e, false);
			return *this;
		}

		Flags& reset() noexcept {
			m_bits.reset();
			return *this;
		}

		[[nodiscard]] bool test(EnumT e) const noexcept {
			return m_bits.test(underlying(e));
		}

		[[nodiscard]] bool all() const noexcept {
			return m_bits.all();
		}

		[[nodiscard]] bool any() const noexcept {
			return m_bits.any();
		}

		[[nodiscard]] bool none() const noexcept {
			return m_bits.none();
		}

		[[nodiscard]] constexpr std::size_t size() const noexcept {
			return m_bits.size();
		}

		[[nodiscard]] std::size_t count() const noexcept {
			return m_bits.count();
		}

		constexpr bool operator[](EnumT e) const {
			return m_bits[underlying(e)];
		}

	private:
		static constexpr UnderlyingT underlying(EnumT e) {
			return static_cast<UnderlyingT>(e);
		}

		std::bitset<underlying(EnumT::COUNT)> m_bits;
	};
}