RWStructuredBuffer<float4> lmo : register(u0, space0);

int main()
{
	lmo[2] = float4(1,1,1,1);
}